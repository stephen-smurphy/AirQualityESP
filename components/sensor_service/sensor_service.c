#include <math.h>
#include "sensor_service.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sgp30_controller.h"
#include "sht3x_controller.h"
#include "i2c_controller.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_timer.h"

#define SGP30_ADDR 0x58
#define SHT3X_ADDR 0x44

#define SENSOR_TASK_PERIOD_MS 1000

static bool load_baseline_from_nvs(sgp30_measurement_t *baseline);
static bool store_baseline_to_nvs(const sgp30_measurement_t *baseline);
static float calculate_absolute_humidity(float temp, float humidity);

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t sgp_handle;
static i2c_master_dev_handle_t sht_handle;

static TaskHandle_t sensor_task_handle;
QueueHandle_t sensor_queue;

static void sensor_task(void *arg) {
    TickType_t last_wake = xTaskGetTickCount();

    static int64_t boot_us = 0;
    static int64_t last_baseline_store_us = 0;
    static bool baseline_training_complete = false;
    static uint8_t shtSampleCount = 10;
    
    if (boot_us == 0) {
        boot_us = esp_timer_get_time();
    }

    for (;;) {
        sht3x_measurement_t sht_measurement;
        sgp30_measurement_t sgp_measurement;

        sensor_data_t data;

        if(shtSampleCount == 10) {
            data.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            if (sht3x_measure(sht_handle, &sht_measurement) == ESP_OK) {
                data.temperature = sht_measurement.temp;
                data.humidity = sht_measurement.humidity;
                sgp30_send_absolute_humidity(sgp_handle, calculate_absolute_humidity(sht_measurement.temp, sht_measurement.humidity));
            }
            if (sgp30_measure(sgp_handle, &sgp_measurement) == ESP_OK) {
                data.eco2 = sgp_measurement.eco2;
                data.tvoc = sgp_measurement.tvoc;
            }

            xQueueOverwrite(sensor_queue, &data);
            shtSampleCount = 1;
        }
        else {
            //SGP30 needs a measurement every second to maintain accuracy, even if we only need a sample every 10 seconds
            sgp30_measure(sgp_handle, &sgp_measurement);
        }
        shtSampleCount++;

        int64_t now_us = esp_timer_get_time();
        int64_t uptime_sec = (now_us - boot_us) / 1000000;

        if (!baseline_training_complete && uptime_sec >= 12 * 3600) {
            baseline_training_complete = true;
        }

        if (baseline_training_complete && (now_us - last_baseline_store_us >= 3600LL * 1000000LL)) {
            sgp30_measurement_t baseline;
            if (sgp30_get_iaq_baseline(sgp_handle, &baseline) == ESP_OK) {
                if(store_baseline_to_nvs(&baseline)) {
                    last_baseline_store_us = now_us;
                }
            }
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

esp_err_t sensor_service_start(void) {
    esp_err_t err = i2c_init_bus(&bus_handle);
    if(err != ESP_OK) return err;

    err = i2c_add_device(&bus_handle, SGP30_ADDR, &sgp_handle);
    if(err != ESP_OK) return err;

    err = i2c_add_device(&bus_handle, SHT3X_ADDR, &sht_handle);
    if(err != ESP_OK) return err;

    err = sgp_init(sgp_handle);
    if(err != ESP_OK) return err;

    //Check for NVS baseline and send to SGP30 if found
    sgp30_measurement_t baseline;
    if(load_baseline_from_nvs(&baseline)) {
        err = sgp30_set_iaq_baseline(sgp_handle, &baseline);
        if(err != ESP_OK) return err;
    }

    err = sht3x_init(sht_handle);
    if(err != ESP_OK) return err;

    sensor_queue = xQueueCreate(1, sizeof(sensor_data_t));
    BaseType_t ok = xTaskCreate(sensor_task, "Sensor Task", 4096, NULL, 5, &sensor_task_handle);

    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

//Calculates and returns the absolute humidity in g/m^3
static float calculate_absolute_humidity(float temp, float humidity) {
    return 216.7f * (((humidity/100.0f) * 6.112f * expf((17.62f * temp) / (243.12f + temp))) / (273.15f + temp));
}

//Checks NVS for baseline CO2 and TVOC values, returns true if the values are loaded into *baseline
static bool load_baseline_from_nvs(sgp30_measurement_t *baseline) {
    if (baseline == NULL) return false;

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return false;
    }

    err = nvs_get_u16(nvs_handle, "co2_baseline", &baseline->eco2);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_get_u16(nvs_handle, "tvoc_baseline", &baseline->tvoc);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}

//Stores the baseline values in the *baseline buffer to NVS, returns true if successful
static bool store_baseline_to_nvs(const sgp30_measurement_t *baseline) {
    if (baseline == NULL) return false;

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return false;
    }

    err = nvs_set_u16(nvs_handle, "co2_baseline", baseline->eco2);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_set_u16(nvs_handle, "tvoc_baseline", baseline->tvoc);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}
