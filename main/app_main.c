#include "sensor_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    sensor_data_t data;
    ESP_ERROR_CHECK(sensor_service_start());

    for(;;) {
        if(get_sensor_service_data(&data)) {
            ESP_LOGI(TAG, "TIMESTAMP: %u Temp: %f Humidity: %f eCO2: %u TVOC: %u", data.timestamp_ms, data.temperature, data.humidity, data.eco2, data.tvoc);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
