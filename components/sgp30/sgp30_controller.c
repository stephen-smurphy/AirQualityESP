#include "sgp30_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_controller.h"

#define SGP30_CMD_INIT 0x2003
#define SGP30_CMD_MEASURE 0x2008
#define SGP_TIMEOUT_MS 100
#define SGP_INIT_WARM_UP_MS 15000
#define SGP_MEASURE_WAIT_MS 20

//Helper function to send commands
static inline esp_err_t sgp30_send_cmd(i2c_master_dev_handle_t dev, uint16_t cmd) {
    uint8_t buf[2] = { cmd >> 8, cmd & 0xFF };
    return i2c_write_to_device(dev, buf, sizeof(buf), pdMS_TO_TICKS(SGP_TIMEOUT_MS));
}

esp_err_t sgp_init(i2c_master_dev_handle_t dev) {
    //Send the init message to the device
    esp_err_t error = sgp30_send_cmd(dev, SGP30_CMD_INIT);
    if(error != ESP_OK) return error;

    //Need to wait 15 seconds for sensor to initialise
    vTaskDelay(pdMS_TO_TICKS(SGP_INIT_WARM_UP_MS));
    return ESP_OK;
}

esp_err_t sgp30_measure(i2c_master_dev_handle_t dev, sgp30_measurement_t *out) {
    //Send measure command, wait for measure to complete (max 12 ms) then read in the measurement
    uint8_t read_buf[6];
    esp_err_t error = sgp30_send_cmd(dev, SGP30_CMD_MEASURE);
    if(error != ESP_OK) return error;

    vTaskDelay(pdMS_TO_TICKS(SGP_MEASURE_WAIT_MS));

    error = i2c_read_from_device(dev, read_buf, sizeof(read_buf), pdMS_TO_TICKS(SGP_TIMEOUT_MS));
    if(error != ESP_OK) return error;

    //Parse measurement
    //TODO Add crc check later

    out->eco2 = (read_buf[0] << 8) | read_buf[1];
    out->tvoc = (read_buf[3] << 8) | read_buf[4];

    return ESP_OK;
}