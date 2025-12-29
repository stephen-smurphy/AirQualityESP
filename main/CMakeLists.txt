idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES sensor_service nvs_flash mqtt_service wifi_service esp_netif
)
