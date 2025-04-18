#include "certs.h"
#include "host.h"

const char *TAG_MQTT = "MQTT_EXAMPLE";
const char *CLIENT_ID = "MQTT_CLIENT_" XSTR(DEV_ID);


void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", message, error_code);
    }
}

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
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    host_t *host = (host_t *)handler_args;
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);

    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    int msg_id;
    char topic[event->topic_len + 1];
    char data[event->data_len + 1];

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        if (IS_CONTROL) {
            esp_mqtt_client_subscribe(client, "device/response/enriched", 1);
        } else {
            esp_mqtt_client_subscribe(client, "device/challenge", 1);
            esp_mqtt_client_subscribe(client, "control/fail", 1);
            esp_mqtt_client_subscribe(client, "control/ok", 1);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
            

        memcpy(topic, event->topic, event->topic_len);
        topic[event->topic_len] = '\0';
        memcpy(data, event->data, event->data_len);
        data[event->data_len] = '\0';

        ESP_LOGI("MQTT", "Received on topic=%s data=%s", topic, data);

        if (strcmp(topic, "device/challenge") == 0) {

           if (!IS_CONTROL) respond_to_challenge(host, data);
        } else if (strcmp(topic, "device/response/enriched") == 0) {

           if (IS_CONTROL) verify_challenge_response(data, host);
        } else if (strcmp(topic, "control/ok") == 0) {

            if (!IS_CONTROL) on_allow_message(data);
        } else if (strcmp(topic, "control/fail") == 0) {
           
            if (!IS_CONTROL) on_allow_message(data);
        } else {

            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {

            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG_MQTT, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(const char* mqtt_broker_url, host_t *host)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = mqtt_broker_url,
        .cert_pem = AWS_ROOT_CA,         // AWS CA
        .client_cert_pem = DEVICE_CERT,  // Device Certificate
        .client_key_pem = PRIV_KEY,       // Private key
        .client_id = CLIENT_ID
    };

    host->mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(host->mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, host);
    esp_mqtt_client_start(host->mqtt_client);
}
