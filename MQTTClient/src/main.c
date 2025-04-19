#include "nvs_flash.h"
#include "host.h"
#include "esp_log.h"
#include "mbedtls/md.h"

const char* Wifi_SSID; 
const char* Wifi_Pass; 
extern const uint8_t shared_key[];
extern const size_t   shared_key_len;


static uint32_t last_nonce_sent = 0;

int is_valid = 0;

// AWS Stuff
#define CONFIG_AWS_IOT_MQTT_TX_BUF_LEN 100
#define CONFIG_AWS_IOT_MQTT_RX_BUF_LEN 100
#define CONFIG_AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 4

const char* Mqtt_Broker_Url;
// Host object
host_t host;

void app_main(void)
{
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  host.wifi_creds.Wifi_SSID = Wifi_SSID;
  host.wifi_creds.Wifi_Pass = Wifi_Pass;

  init_host(&host);

  mqtt_app_start(Mqtt_Broker_Url, &host);

  if(IS_CONTROL) xTaskCreate(challenge_task, "challenge_task", 4096, &host, 5, NULL);
  while(1)
  {
    if (is_valid) {
    send_env_data(&host);
    }
    vTaskDelay(MS2TICK(5000));
  }
}

void on_allow_message(const char* pl) {
  uint8_t node_id;
  uint16_t port;
  char ip[40];
  if (sscanf(pl, "%03hhu:OK:%s:%hu", &node_id, ip, &port) == 3 && node_id == host.aws_mqtt_id) {

    is_valid = 1;
  } else if (sscanf(pl, "%03hhu:FAIL", &node_id) == 1 && node_id == host.aws_mqtt_id) {

    is_valid = 0;
  }
}

void send_env_data(host_t* host)
{
  int msg_id;
  char buff[40];
  htu21_data_t env_data;
  if(host->htu21.msg_queue == NULL)
  {
    LOG_ERROR("MSG queue does not exist cannot send enviornment data.");
  }

  xQueuePeek(host->htu21.msg_queue, &env_data, 5);
  sprintf(buff, "{\"temperature\": %.2f, \"humidity\": %.2f}", env_data.temperature, env_data.humidity);
  msg_id = esp_mqtt_client_publish(host->mqtt_client, "data/sensor", buff, 0, 1, 0);

  LOG_PRINTF("Sent publish successful, msg_id=%d", msg_id);
}
#define HMAC_LEN 32  // SHA‑256 output size

static bool hmac_equal(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

static void hex2bin(const char *hex, uint8_t *out) {
    for (int i = 0; i < HMAC_LEN; i++) {
        uint8_t hi = (hex[2*i] <= '9' ? hex[2*i] - '0' : (hex[2*i]&~32) - 'A' + 10);
        uint8_t lo = (hex[2*i+1] <= '9' ? hex[2*i+1] - '0' : (hex[2*i+1]&~32) - 'A' + 10);
        out[i] = (hi << 4) | lo;
    }
}

static void compute_hmac_sha256(uint32_t nonce, uint8_t out_hmac[HMAC_LEN]) {
    uint8_t nonce_be[4] = {
        (nonce >> 24) & 0xFF,
        (nonce >> 16) & 0xFF,
        (nonce >>  8) & 0xFF,
        (nonce      ) & 0xFF,
    };

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, info, /* HMAC = */ 1);
    mbedtls_md_hmac_starts(&ctx, shared_key, shared_key_len);
    mbedtls_md_hmac_update(&ctx, nonce_be, sizeof(nonce_be));
    mbedtls_md_hmac_finish(&ctx, out_hmac);
    mbedtls_md_free(&ctx);
}


uint32_t compute_response(uint32_t nonce) {
    return nonce ^ shared_key;
}

void publish_challenge(host_t* host) {
    last_nonce_sent = esp_random();  // Generate a random 32-bit nonce

    char challenge_msg[32];

    sprintf(challenge_msg, "%03d:CHALLENGE:%08X", host->aws_mqtt_id, last_nonce_sent);
    int msg_id = esp_mqtt_client_publish(host->mqtt_client, "device/challenge", challenge_msg, 0, 1, 0);
    ESP_LOGI("CHALLENGE", "Published challenge: %s, msg_id=%d", challenge_msg, msg_id);

}


void challenge_task(void *pvParameter) {
    host_t* host = (host_t*)pvParameter;
    while (1) {
        publish_challenge(host);
        vTaskDelay(pdMS_TO_TICKS(CHALLENGE_INTERVAL_MS));
    }
}

void respond_to_challenge(host_t* host, const char* payload) {
    uint32_t received_nonce;
    uint8_t  aws_id;

    if (sscanf(payload, "%03hhu:CHALLENGE:%08X", &aws_id, &received_nonce) == 2) {
        uint8_t hmac[HMAC_LEN];
        compute_hmac_sha256(received_nonce, hmac);

        // hex‑encode
        char hmac_hex[HMAC_LEN*2 + 1];
        for (int i = 0; i < HMAC_LEN; i++) {
            sprintf(&hmac_hex[i*2], "%02X", hmac[i]);
        }
        hmac_hex[HMAC_LEN*2] = '\0';

        char resp_msg[64];
        // only ID and HMAC
        sprintf(resp_msg, "%03hhu:RESPONSE:%s", host->aws_mqtt_id, hmac_hex);
        esp_mqtt_client_publish(
            host->mqtt_client,
            "device/response",
            resp_msg, 0, 1, 0
        );
        ESP_LOGI("CHALLENGE", "Sent HMAC response: %s", resp_msg);
    } else {
        ESP_LOGE("CHALLENGE", "Bad challenge format: %s", payload);
    }
}

void verify_challenge_response(const char* resp_payload, host_t* host) {
    uint8_t aws_id;
    uint16_t port;
    char ip[40]
    char    recv_hmac_hex[HMAC_LEN*2 + 1];

    if (sscanf(resp_payload, "%03hhu:RESPONSE:%64s:%s:%hu", &aws_id, recv_hmac_hex, ip, &port) == 2) {
        uint8_t recv_hmac[HMAC_LEN], exp_hmac[HMAC_LEN];
        hex2bin(recv_hmac_hex, recv_hmac);
        compute_hmac_sha256(last_nonce_sent, exp_hmac);

        if (hmac_equal(exp_hmac, recv_hmac, HMAC_LEN)) {
            ESP_LOGI("CHALLENGE", "HMAC verified OK");
            // proceed: publish control/ok...
            char ok_msg[100];
            sprintf(ok_msg, "%03hhu:OK:%s:%hu", aws_id, ip, port);
            esp_mqtt_client_publish(host->mqtt_client, "control/ok",
                                    ok_msg, 0, 1, 0);
        } else {
            ESP_LOGW("CHALLENGE", "HMAC mismatch");
            char fail_msg[100];
            sprintf(fail_msg, "%03hhu:FAIL:%s:%hu", aws_id, ip, port);
            esp_mqtt_client_publish(host->mqtt_client, "control/fail",
                                    fail_msg, 0, 1, 0);
        }
    } else {
        ESP_LOGE("CHALLENGE", "Bad response format: %s", resp_payload);
    }
}

