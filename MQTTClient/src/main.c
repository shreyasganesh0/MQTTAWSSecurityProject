#include "nvs_flash.h"
#include "host.h"
#include "esp_log.h"

const char* Wifi_SSID; 
const char* Wifi_Pass; 
#const uint32_t shared_key;

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

void respond_to_challenge(host_t* host, const char* challenge_payload) {
    uint32_t received_nonce;
    uint8_t aws_sender_id;

    if (sscanf(challenge_payload, "%03hhu:CHALLENGE:%08X", &aws_sender_id, &received_nonce) == 2) {

        uint32_t response_val = compute_response(received_nonce);
        char response_msg[32];
        sprintf(response_msg, "%03hhu:RESPONSE:%08X", host->aws_mqtt_id, response_val);
        int msg_id = esp_mqtt_client_publish(host->mqtt_client, "device/response", response_msg, 0, 1, 0);

        ESP_LOGI("CHALLENGE", "Responded to challenge %08X with response: %s, msg_id=%d", received_nonce, response_msg, msg_id);
    } else {
        ESP_LOGE("CHALLENGE", "Invalid challenge format received: %s", challenge_payload);
    }
}

void verify_challenge_response(const char* response_payload, host_t *host) {
    uint32_t received_response;
    uint8_t aws_id;
    uint16_t port; 
    char ip[40];
    char response_msg[102];

    if (sscanf(response_payload, "%03hhu:RESPONSE:%08X:%s:%hu", &aws_id, &received_response, ip, &port) == 4) {
        uint32_t expected_response = compute_response(last_nonce_sent);
        if (received_response == expected_response) {

            sprintf(response_msg, "%03hhu:OK:%s:%hu", aws_id, ip, port);
            ESP_LOGE("CHALLENGE", "Challenge response verified successfully: %08X", received_response);
            int msg_id = esp_mqtt_client_publish(host->mqtt_client, "control/ok", response_msg, 0, 1, 0);
        } else {

            ESP_LOGE("CHALLENGE", "Challenge response verification failed. Expected %08X, got %08X", expected_response, received_response);
            sprintf(response_msg, "%03hhu:FAIL", aws_id);
            int msg_id = esp_mqtt_client_publish(host->mqtt_client, "control/fail", response_msg, 0, 1, 0);
        }
    } else {
        ESP_LOGE("CHALLENGE", "Invalid response format received: %s", response_payload);
    }
}
