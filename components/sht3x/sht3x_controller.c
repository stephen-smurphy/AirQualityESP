#include "sht3x_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_controller.h"

#define SHT3X_CMD_RESET 0x30A2 
#define SHT3X_CMD_MEASURE 0x2400
#define SHT_TIMEOUT_MS 100
#define SHT_INIT_WARM_UP_MS 20
#define SHT_MEASURE_WAIT_MS 20

//Helper function to send commands
static inline esp_err_t sht3x_send_cmd(i2c_master_dev_handle_t dev, uint16_t cmd) {
    uint8_t buf[2] = { cmd >> 8, cmd & 0xFF };
    return i2c_write_to_device(dev, buf, sizeof(buf), pdMS_TO_TICKS(SHT_TIMEOUT_MS));
}

esp_err_t sht3x_init(i2c_master_dev_handle_t dev) {
    esp_err_t error = sht3x_send_cmd(dev, SHT3X_CMD_RESET);
    if(error != ESP_OK) return error;

    //Small delay for sensor to initialise
    vTaskDelay(pdMS_TO_TICKS(SHT_INIT_WARM_UP_MS));
    return ESP_OK;
}

esp_err_t sht3x_measure(i2c_master_dev_handle_t dev, sht3x_measurement_t *out) {
    //Send measure command, wait for measure to complete (max 12 ms) then read in the measurement
    uint8_t read_buf[6];
    esp_err_t error = sht3x_send_cmd(dev, SHT3X_CMD_MEASURE);
    if(error != ESP_OK) return error;

    vTaskDelay(pdMS_TO_TICKS(SHT_MEASURE_WAIT_MS));

    error = i2c_read_from_device(dev, read_buf, sizeof(read_buf), pdMS_TO_TICKS(SHT_TIMEOUT_MS));
    if(error != ESP_OK) return error;

    //Parse measurement
    //TODO Add crc check later

    uint16_t raw_temp = (read_buf[0] << 8) | read_buf[1];
    uint16_t raw_humidity = (read_buf[3] << 8) | read_buf[4];

    out->temp = (-45.0f + 175.0f * (((float)raw_temp) / 65535.0f));
    out->humidity = (100.0f * ((float)raw_humidity / 65535.0f));

    return ESP_OK;
}