#include "wifi_service.h"

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_err.h"

#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "wifi_service";

static EventGroupHandle_t wifi_event_group;
static bool connected = false;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Wi-Fi started, connecting...");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "Wi-Fi disconnected, retrying...");
                connected = false;
                esp_wifi_connect();
                break;

            default:
                break;
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        connected = true;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_service_start(void) {
    esp_err_t err = esp_netif_init();
    if(err != ESP_OK) return err;

    err = esp_event_loop_create_default();
    if(err != ESP_OK) return err;

    wifi_event_group = xEventGroupCreate();
    if (!wifi_event_group) {
        return ESP_ERR_NO_MEM;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if(err != ESP_OK) return err;

    err = esp_event_handler_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL
    );
    if(err != ESP_OK) return err;

    err = esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL
    );
    if(err != ESP_OK) return err;

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if(err != ESP_OK) return err;

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if(err != ESP_OK) return err;

    err = esp_wifi_start();
    if(err != ESP_OK) return err;

    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    ESP_LOGI(TAG, "SSID: %s Password: %s", wifi_config.sta.ssid, wifi_config.sta.password);

    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    ESP_LOGI(TAG, "Wi-Fi connected");

    return ESP_OK;
}

bool wifi_is_connected() {
    return connected;
}
