#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "driver/i2c_master.h"

typedef struct {
    uint16_t eco2;
    uint16_t tvoc;
} sgp30_measurement_t;

esp_err_t sgp_init(i2c_master_dev_handle_t dev);
esp_err_t sgp30_measure(i2c_master_dev_handle_t dev, sgp30_measurement_t *out);