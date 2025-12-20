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

esp_err_t sensor_service_start();

bool get_sensor_service_data(sensor_data_t *data);
