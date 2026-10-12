idf_component_register(
    SRCS "ota_client.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES wifi_link settings esp_https_ota esp_http_client json esp_app_format esp_timer mbedtls app_update
)
