idf_component_register(
    SRCS "ota_service.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_http_client esp_https_ota app_update freertos log
)
