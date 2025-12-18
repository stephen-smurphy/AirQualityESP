#include <stdio.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_controller.h"
#include "sgp30_controller.h"
#include "sht3x_controller.h"

#define SGP30_ADDR 0x58
#define SHT3X_ADDR 0x44

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t sgp_handle;
    i2c_master_dev_handle_t sht_handle;

    esp_err_t err;

    //Initialise the bus
    err = i2c_init_bus(&bus_handle);
    if(err == ESP_OK) {
        ESP_LOGI(TAG, "I2C Initialized Successfully");
    }
    else {
        ESP_LOGE(TAG, "Failed to intialise bus (err=%s)", esp_err_to_name(err));
        abort();
    }

    err = i2c_add_device(&bus_handle, SGP30_ADDR, &sgp_handle);
    if(err == ESP_OK) {
        ESP_LOGI(TAG, "SGP30 Added to Bus Successfully");
    }
    else {
        ESP_LOGE(TAG, "Failed to add SGP30 (err=%s)", esp_err_to_name(err));
        abort();
    }

    err = i2c_add_device(&bus_handle, SHT3X_ADDR, &sht_handle);
    if(err == ESP_OK) {
        ESP_LOGI(TAG, "SHT3X Added to Bus Successfully");
    }
    else {
        ESP_LOGE(TAG, "Failed to add SHT3X (err=%s)", esp_err_to_name(err));
        abort();
    }

    err = sgp_init(sgp_handle);
    if(err == ESP_OK) {
        ESP_LOGI(TAG, "SGP30 Initialized");
    }
    else {
        ESP_LOGE(TAG, "Failed to init SGP30 (err=%s)", esp_err_to_name(err));
        abort();
    }

    err = sht3x_init(sht_handle);
    if(err == ESP_OK) {
        ESP_LOGI(TAG, "SHT3X Initialized");
    }
    else {
        ESP_LOGE(TAG, "Failed to init SHT3X (err=%s)", esp_err_to_name(err));
        abort();
    }
    
    while (1) {
        //Send measure command
        sgp30_measurement_t sgp_measurement;
        sht3x_measurement_t sht_measurement;

        err = sgp30_measure(sgp_handle, &sgp_measurement);
        if(err == ESP_OK) {
            ESP_LOGI(TAG, "CO2eq: %u", sgp_measurement.eco2);
            ESP_LOGI(TAG, "TVOC: %u", sgp_measurement.tvoc);
        }
        else {
            ESP_LOGE(TAG, "Failed to get measurement from SGP30 (err=%s)", esp_err_to_name(err));
            abort();
        }

        err = sht3x_measure(sht_handle, &sht_measurement);
        if(err == ESP_OK) {
            ESP_LOGI(TAG, "Temp: %f", sht_measurement.temp);
            ESP_LOGI(TAG, "Humidity: %f\n", sht_measurement.humidity);
        }
        else {
            ESP_LOGE(TAG, "Failed to get measurement from SHT3X (err=%s)", esp_err_to_name(err));
            abort();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
