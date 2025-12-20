#include "sensor_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    sensor_data_t data;

    sensor_service_start();

    for(;;) {
        if(get_sensor_service_data(&data)) {
            ESP_LOGI(TAG, "TIMESTAMP: %u Temp: %f Humidity: %f eCO2: %u TVOC: %u", data.timestamp_ms, data.temperature, data.humidity, data.eco2, data.tvoc);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
