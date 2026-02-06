#include <string.h>
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_https_ota.h"
#include "ota_service.h"

static const char *TAG = "OTA";

#define FW_VERSION "1.0.0"

/**
* @brief Retreives a version number from a version.txt file on the OTA webserver 
*        and checks if the current version is different to the available OTA firmware version
*
* @return esp_err_t The esp error code
*/
static esp_err_t check_for_updates() {
    char latest_version[16];

    //Uses HTTP only for testing, for production this should use HTTPS and TLS certificates
    esp_http_client_config_t config = {
        .url = CONFIG_OTA_UPDATE_VERSION_URL,
        .method = HTTP_METHOD_GET,
        .transport_type = HTTP_TRANSPORT_OVER_TCP
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init client");
        return ESP_FAIL;
    } 

    esp_err_t err = esp_http_client_open(client, 0);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int64_t status = esp_http_client_fetch_headers(client);
    if(status < 0) {
        ESP_LOGE(TAG, "Failed to fetch headers");
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    //Reads the payload into the latest_version array and appends a null character to make it a valid C-String
    int len = esp_http_client_read(client, latest_version, sizeof(latest_version) - 1);
    if (len > 0) {
        latest_version[len] = '\0';
    } 
    else {
        ESP_LOGW(TAG, "Failed to read version.txt, len=%d", len);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Current FW: %s, Latest FW: %s", FW_VERSION, latest_version);

    esp_http_client_cleanup(client);

    //Check if current firmware version matches the version on the server, no need to update if they are the same
    if (strcmp(FW_VERSION, latest_version) != 0) {
        return ESP_OK;
    }
    return ESP_FAIL;
}

/**
* @brief The OTA service freeRTOS task
*
*/
void ota_task(void *arg) {
    //Uses HTTP only for testing, for production this should use HTTPS and TLS certificates
    esp_http_client_config_t http_config = {
        .url = CONFIG_OTA_UPDATE_FIRMWARE_URL,
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
