idf_component_register(
    SRCS "app_main.c"
    REQUIRES sensor_service nvs_flash mqtt_service wifi_service led_service ota
)
