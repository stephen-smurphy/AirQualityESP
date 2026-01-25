idf_component_register(
    SRCS "mqtt_service.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES sensor_service mqtt led_service
)
