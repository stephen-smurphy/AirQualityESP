/**
* @file sgp30.h
* @brief Driver interface for the SGP30 air quality sensor
*
* Provides initialisation and measurement functions for the Sensirion
* SGP30 gas sensor, allowing retrieval of eCO₂ and TVOC measurements
* over an I2C interface
*/

#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "driver/i2c_master.h"

/**
* @brief SGP30 air quality measurement data
*/
typedef struct {
    uint16_t eco2;
    uint16_t tvoc;
} sgp30_measurement_t;

/**
* @brief Initializes the SGP30 sensor
*
* @param dev I2C device handle for the SGP30
* @return esp_err_t ESP error code
*/
esp_err_t sgp_init(i2c_master_dev_handle_t dev);

/**
 * @brief Reads an air quality measurement from the SGP30 sensor
 *
 * @param dev I2C device handle for the SGP30
 * @param out Pointer to a structure that receives the measurement data
 * @return esp_err_t ESP error code
 */
esp_err_t sgp30_measure(i2c_master_dev_handle_t dev, sgp30_measurement_t *out);

/**
 * @brief Sends an absolute humidity reading to the sgp30 to use for more accurate co2 and tvoc measurements
 *
 * @param dev I2C device handle for the SGP30
 * @param absolute_humidity Float value of the humidity in g/m^3
 * @return esp_err_t ESP error code
 */
esp_err_t sgp30_send_absolute_humidity(i2c_master_dev_handle_t dev, float absolute_humidity);
