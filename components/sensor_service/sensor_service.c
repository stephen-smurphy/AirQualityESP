#include "sensor_service.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sgp30_controller.h"
#include "sht3x_controller.h"
#include "i2c_controller.h"

#define SGP30_ADDR 0x58
#define SHT3X_ADDR 0x44

#define SENSOR_TASK_PERIOD_MS 1000

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t sgp_handle;
static i2c_master_dev_handle_t sht_handle;

static TaskHandle_t sensor_task_handle;
static QueueHandle_t sensor_queue;

static void sensor_task(void *arg) {
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        sht3x_measurement_t sht_measurement;
        sgp30_measurement_t sgp_measurement;

        sensor_data_t data;
        data.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (sht3x_measure(sht_handle, &sht_measurement) == ESP_OK) {
            data.temperature = sht_measurement.temp;
            data.humidity = sht_measurement.humidity;
        }

        if (sgp30_measure(sgp_handle, &sgp_measurement) == ESP_OK) {
            data.eco2 = sgp_measurement.eco2;
            data.tvoc = sgp_measurement.tvoc;
        }
        
        xQueueOverwrite(sensor_queue, &data);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

esp_err_t sensor_service_start() {
    esp_err_t err;

    err = i2c_init_bus(&bus_handle);
    if(err != ESP_OK) return err;

    err = i2c_add_device(&bus_handle, SGP30_ADDR, &sgp_handle);
    if(err != ESP_OK) return err;

    err = i2c_add_device(&bus_handle, SHT3X_ADDR, &sht_handle);
    if(err != ESP_OK) return err;

    err = sgp_init(sgp_handle);
    if(err != ESP_OK) return err;

    err = sht3x_init(sht_handle);
    if(err != ESP_OK) return err;

    sensor_queue = xQueueCreate(1, sizeof(sensor_data_t));
    BaseType_t ok = xTaskCreate(sensor_task, "Sensor Task", 4096, NULL, 5, &sensor_task_handle);

    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

bool get_sensor_service_data(sensor_data_t *data) {
    if (!sensor_queue) return false;

    return xQueueReceive(sensor_queue, data, 0) == pdPASS;
}
