#include "sensor_service.h"
#include "mqtt_service.h"
#include "esp_event.h"
#include "wifi_service.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static esp_err_t init_nvs(void);

void app_main(void)
{
    ESP_ERROR_CHECK(init_nvs());

    ESP_ERROR_CHECK(wifi_service_start());

    ESP_ERROR_CHECK(sensor_service_start());

    ESP_ERROR_CHECK(mqtt_service_start());
}

static esp_err_t init_nvs(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase();
        if (err != ESP_OK) return err;
        err = nvs_flash_init();
        return err;
    }
    return err;
}
