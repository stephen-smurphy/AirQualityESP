/**
* @file sensor_service.h
* @brief The high level sensor service that controls the measurement of data from the connected sensors
* 
*/

#pragma once

#include <stdint.h>
#include "stdbool.h"
#include "esp_err.h"

typedef struct {
    float temperature;
    float humidity;
    uint32_t eco2;
    uint32_t tvoc;
    uint32_t timestamp_ms;
} sensor_data_t;

/**
* @brief Initialises the sensors and the i2c bus and starts the FreeRTOS sensor measurement task
*
* @return esp_err_t The esp error code
*/
esp_err_t sensor_service_start(void);
