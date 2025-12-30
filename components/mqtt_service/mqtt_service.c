#include "mqtt_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "MQTT";

extern QueueHandle_t sensor_queue;

static esp_mqtt_client_handle_t client = NULL;
static TaskHandle_t wifi_mqtt_task_handle;
static bool connected = false;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        connected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        connected = false;
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void wifi_mqtt_task(void *arg) {
    sensor_data_t data;
    uint32_t last_timestamp = 0;
    for (;;) {
        if (xQueueReceive(sensor_queue, &data, portMAX_DELAY) == pdPASS) {
            if(connected) {
                if(data.timestamp_ms != last_timestamp) {
                    last_timestamp = data.timestamp_ms;
                    char payload[128];
                    // Convert sensor data to string JSON or plain
                    snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f, \"eco2\": %lu, \"tvoc\": %lu}",
                            data.temperature,
                            data.humidity,
                            (unsigned long)data.eco2,
                            (unsigned long)data.tvoc);
                    esp_mqtt_client_publish(client, "AirQuality", payload, 0, 1, 0);
                }
            } else {
                ESP_LOGW(TAG, "MQTT not connected, dropping data");
            }
        }
    }
}

esp_err_t mqtt_service_start() {
     esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_URI,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) return ESP_ERR_NO_MEM;

    esp_err_t err = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (err != ESP_OK) return err;

    err = esp_mqtt_client_start(client);
    if (err != ESP_OK) return err;

    BaseType_t ok = xTaskCreate(wifi_mqtt_task, "Wifi MQTT Task", 4096, NULL, 5, &wifi_mqtt_task_handle);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

bool mqtt_client_connected(void) {
    return connected;
}
