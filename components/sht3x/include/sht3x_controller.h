/**
* @file sgp30.h
* @brief Driver interface for the SHT3X temperature and humidity sensor
*
* Provides initialisation and measurement functions for the Sensirion
* SHT3X sensor, allowing retrieval of temperature and humidity measurements
* over an I2C interface
*/

#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "driver/i2c_master.h"

/**
* @brief SHT3X temperature and humidity measurement data
*/
typedef struct {
    float temp;
    float humidity;
} sht3x_measurement_t;

/**
* @brief Initializes the SHT3X sensor
*
* @param dev I2C device handle for the SHT3X
* @return esp_err_t ESP error code
*/
esp_err_t sht3x_init(i2c_master_dev_handle_t dev);

/**
 * @brief Reads a measurement from the SHT3X sensor
 *
 * @param dev I2C device handle for the SHT3X
 * @param out Pointer to a structure that receives the measurement data
 * @return esp_err_t ESP error code
 */
esp_err_t sht3x_measure(i2c_master_dev_handle_t dev, sht3x_measurement_t *out);