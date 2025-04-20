#ifndef __HOST_H__
#define __HOST_H__

/***************************************************************************************************/
/* Include Files */
/***************************************************************************************************/
// WiFi
#include "wifi.h"
// I2C devices
#include "oled.h"
#include "htu21d.h"
#include "driver/i2c.h"
#include "i2c.h"
// Console
#include "driver/uart.h"
#include "serial.h"
#include "console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "mqtt_client.h"
#define STR(s) #s
#define XSTR(s) STR(s)

#ifndef DEV_ID
#define DEV_ID 001
#endif

#ifndef IS_CONTROL
#define IS_CONTROL 0
#endif

#define CHALLENGE_INTERVAL_MS (120000)

typedef struct {
    unsigned char* Wifi_SSID;
    unsigned char* Wifi_Pass;
} wifi_creds_t;

typedef struct {
    uint8_t aws_mqtt_id;
    console_t console;
    htu21_t htu21;
    ssd1306_t ssd1306;
    wifi_creds_t wifi_creds;
    esp_mqtt_client_handle_t mqtt_client;
} host_t;


void send_env_data(host_t* host);
void mqtt_app_start(const char* mqtt_broker_url, host_t *host);

void on_allow_message(const char* pl); 
void log_error_if_nonzero(const char *message, int error_code);
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_app_start(const char* mqtt_broker_url, host_t *host);
void init_host(host_t* );
void verify_challenge_response(const char* response_payload, host_t *host); 
void respond_to_challenge(host_t* host, const char* challenge_payload); 
void challenge_task(void *pvParameter);
void publish_challenge(host_t* host); 
uint32_t compute_nonce_xor(uint32_t nonce);

#endif /* __HOST_H__ */
