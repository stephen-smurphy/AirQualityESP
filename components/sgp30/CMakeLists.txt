idf_component_register(
    SRCS "sgp30_controller.c"
    INCLUDE_DIRS "include"
    REQUIRES driver i2c crc8
)
