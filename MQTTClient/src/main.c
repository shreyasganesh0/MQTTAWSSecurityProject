#include "nvs_flash.h"
#include "host.h"
#include "esp_log.h"
#include "mbedtls/md.h"

const char* Wifi_SSID = "SETUP-1CCC"; 
const char* Wifi_Pass = "center1832block"; 
const uint32_t shared_key = 0x8DEEF1;
const size_t shared_key_len = sizeof(shared_key);


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

uint32_t compute_nonce_xor(uint32_t nonce) {
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
        uint32_t comped = compute_nonce_xor(received_nonce);

        char resp_msg[64];
        sprintf(resp_msg, "%03hhu:RESPONSE:%08X", host->aws_mqtt_id, comped);
        esp_mqtt_client_publish(
            host->mqtt_client,
            "device/response",
            resp_msg, 0, 1, 0
        );
    } else {
        ESP_LOGE("CHALLENGE", "Bad challenge format: %s", payload);
    }
}

void verify_challenge_response(const char* resp_payload, host_t* host) {
    uint8_t aws_id;
    uint16_t port;
    uint32_t recv_comp;
    char ip[40];

    if (sscanf(resp_payload, "%03hhu:RESPONSE:%08X:%s:%hu", &aws_id, &recv_comp, ip, &port) == 3) {
        uint32_t comped_v = compute_nonce_xor(last_nonce_sent);

        if (comped_v == recv_comp) {
            char ok_msg[100];
            sprintf(ok_msg, "%03hhu:OK:%s:%hu", aws_id, ip, port);
            esp_mqtt_client_publish(host->mqtt_client, "control/ok",
                                    ok_msg, 0, 1, 0);
        } else {
            char fail_msg[100];
            sprintf(fail_msg, "%03hhu:FAIL:%s:%hu", aws_id, ip, port);
            esp_mqtt_client_publish(host->mqtt_client, "control/fail",
                                    fail_msg, 0, 1, 0);
        }
    } else {
        ESP_LOGE("CHALLENGE", "Bad response format: %s", resp_payload);
    }
}

