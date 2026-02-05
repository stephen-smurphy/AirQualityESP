#include <string.h>
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_https_ota.h"
#include "ota_service.h"

static const char *TAG = "OTA";

#define FW_VERSION "1.0.0"

static esp_err_t check_for_updates() {
    char latest_version[16];

    esp_http_client_config_t config = {
        .url = "http://192.168.1.7:8000/version.txt", //TODO REPLACE
        .method = HTTP_METHOD_GET,
        .transport_type = HTTP_TRANSPORT_OVER_TCP
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return ESP_FAIL;

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        esp_http_client_cleanup(client);
        return ESP_ERR_HTTP_BASE;
    } 

    int len = esp_http_client_read_response(client, latest_version, sizeof(latest_version) - 1);
    if (len >= 0) {
        latest_version[len] = '\0';
        char *newline = strpbrk(latest_version, "\r\n");
        if (newline) *newline = '\0';
    } 
    else {
        ESP_LOGW(TAG, "Failed to read version.txt, len=%d", len);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Current FW: %s, Latest FW: %s", FW_VERSION, latest_version);

    esp_http_client_cleanup(client);

    if (strcmp(FW_VERSION, latest_version) != 0) {
        return ESP_OK;
    }
    return ESP_FAIL;
}

void ota_task(void *arg) {
    esp_http_client_config_t http_config = {
        .url = "http://192.168.1.7:8000/firmware.bin",
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_TCP
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    if(check_for_updates() == ESP_OK) {
        ESP_LOGI(TAG, "Update Found");
        esp_err_t err = esp_https_ota(&ota_config);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "OTA successful, restarting...");
            esp_restart();
        } else {
            ESP_LOGE(TAG, "OTA failed, err=%d", err);
        }
    }

    vTaskDelete(NULL);
}

esp_err_t ota_service_start() {
    ESP_LOGI(TAG, "Starting OTA Task");
    BaseType_t ok = xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}