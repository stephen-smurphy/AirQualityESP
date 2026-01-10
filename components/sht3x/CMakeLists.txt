idf_component_register(
    SRCS "sht3x_controller.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
    PRIV_REQUIRES i2c crc8
)
