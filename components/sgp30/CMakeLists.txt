idf_component_register(
    SRCS "sgp30_controller.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
    PRIV_REQUIRES i2c crc8
)
