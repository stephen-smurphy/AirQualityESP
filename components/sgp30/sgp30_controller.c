#include "sgp30_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_controller.h"
#include "crc8.h"

#define SGP30_CMD_INIT 0x2003
#define SGP30_CMD_MEASURE 0x2008
#define SGP30_CMD_SET_ABSOLUTE_HUMIDITY 0x2061
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

    //Parse measurement and check crc

    uint8_t eco2_crc = read_buf[2];
    uint8_t tvoc_crc = read_buf[5];

    if(eco2_crc != crc8(&read_buf[0], 2) || tvoc_crc != crc8(&read_buf[3], 2)) {
        return ESP_ERR_INVALID_CRC;
    }

    out->eco2 = (read_buf[0] << 8) | read_buf[1];
    out->tvoc = (read_buf[3] << 8) | read_buf[4];

    return ESP_OK;
}

esp_err_t sgp30_send_absolute_humidity(i2c_master_dev_handle_t dev, float absolute_humidity) {
    if (absolute_humidity < 1.0f/256.0f) absolute_humidity = 1.0f/256.0f; // min
    if (absolute_humidity > 255.99609375f) absolute_humidity = 255.99609375f; // max

    //Need to send the humidity as the CMD (2 bytes) + the payload (2 bytes + 1 crc byte)
    uint8_t out[5];
    
    //Convert float to fixed point bytes
    //Example: 0x0F80 corresponds to a humidity value of 15.50 g/m3 (15 g/m3 + 128/256 g/m3)
    uint16_t humidity_bytes = (uint16_t)(absolute_humidity * 256.0f + 0.5f); 

    out[0] = SGP30_CMD_SET_ABSOLUTE_HUMIDITY >> 8;
    out[1] = SGP30_CMD_SET_ABSOLUTE_HUMIDITY & 0xFF;
    out[2] = humidity_bytes >> 8;
    out[3] = humidity_bytes & 0xFF;
    out[4] = crc8(&out[2], 2);

    return i2c_write_to_device(dev, out, sizeof(out), pdMS_TO_TICKS(SGP_TIMEOUT_MS));
}
