#include "sensor_service.h"
#include "mqtt_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "wifi_service.h"
#include "esp_netif.h"

#include "nvs_flash.h"
#include "nvs.h"

void app_main(void)
{
    //Initialise non-volatile storage for baseline sensor value storage
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(wifi_service_start());

    ESP_ERROR_CHECK(sensor_service_start());

    ESP_ERROR_CHECK(mqtt_service_start());
    
}
