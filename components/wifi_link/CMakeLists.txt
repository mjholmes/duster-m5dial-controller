idf_component_register(
    SRCS "wifi_link.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES health esp_wifi esp_netif esp_event nvs_flash console esp_timer
)
