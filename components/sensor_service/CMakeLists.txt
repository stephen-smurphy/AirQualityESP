idf_component_register(
    SRCS "sensor_service.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES driver i2c sgp30 sht3x esp_timer nvs_flash
)
