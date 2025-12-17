#pragma once

#include <stdint.h>
#include <stddef.h>

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

//Init Bus
esp_err_t i2c_init_bus(i2c_master_bus_handle_t *bus_handle);

//Add Device
esp_err_t i2c_add_device(i2c_master_bus_handle_t *bus_handle, uint16_t device_address, i2c_master_dev_handle_t *dev_handle);

//Write to Device
esp_err_t i2c_write_to_device(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buf, size_t size, TickType_t timeout);

//Read from Device
esp_err_t i2c_read_from_device(i2c_master_dev_handle_t dev_handle, uint8_t *read_buf, size_t size, TickType_t timeout);