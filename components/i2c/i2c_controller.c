#include "i2c_controller.h"

#define I2C_MASTER_SCL_IO   22          /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO   21          /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM      -1          /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ  100000      /*!< I2C master clock frequency */

esp_err_t i2c_init_bus(i2c_master_bus_handle_t *bus_handle) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    return i2c_new_master_bus(&bus_config, bus_handle);
}

esp_err_t i2c_add_device(i2c_master_bus_handle_t *bus_handle, uint16_t device_address, i2c_master_dev_handle_t *dev_handle) {
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_address,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    return i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle);
}

esp_err_t i2c_write_to_device(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buf, size_t size, TickType_t timeout) {
    if(!write_buf || size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    return i2c_master_transmit(dev_handle, write_buf, size, timeout);
}

esp_err_t i2c_read_from_device(i2c_master_dev_handle_t dev_handle, uint8_t *read_buf, size_t size, TickType_t timeout) {
    if(!read_buf || size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    return i2c_master_receive(dev_handle, read_buf, size, timeout);
}
