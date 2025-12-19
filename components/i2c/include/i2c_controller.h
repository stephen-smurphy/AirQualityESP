/**
* @file i2c_bus.h
* @brief High-level I2C bus and device helper functions.
* 
*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

/**
* @brief Initialises the I2C bus
*
* @param bus_handle Pointer to the I2C bus handle
* @return esp_err_t The esp error code
*/
esp_err_t i2c_init_bus(i2c_master_bus_handle_t *bus_handle);

/**
* @brief Adds a device to the I2C bus
*
* @param bus_handle Pointer to the I2C bus handle
* @param device_address The I2C address of the device to be added
* @param dev_handle Pointer to the I2C device handle
* @return esp_err_t The esp error code
*/
esp_err_t i2c_add_device(i2c_master_bus_handle_t *bus_handle, uint16_t device_address, i2c_master_dev_handle_t *dev_handle);

/**
* @brief Writes to an I2C device on the bus from a buffer
*
* @param dev_handle Pointer to the I2C device handle
* @param write_buf Pointer to the buffer containing the data to write
* @param size Size of the write buffer
* @param timeout The timeout value in FreeRTOS ticks for the acknowledgement
* @return esp_err_t The esp error code
*/
esp_err_t i2c_write_to_device(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buf, size_t size, TickType_t timeout);

/**
* @brief Reads bytes from an I2C device on the bus into a buffer
*
* @param dev_handle Pointer to the I2C device handle
* @param read_buf Pointer to the buffer to take the bytes read from the device
* @param size Size of the expected bytes to be read
* @param timeout The timeout value in FreeRTOS ticks for the acknowledgement
* @return esp_err_t The esp error code
*/
esp_err_t i2c_read_from_device(i2c_master_dev_handle_t dev_handle, uint8_t *read_buf, size_t size, TickType_t timeout);
