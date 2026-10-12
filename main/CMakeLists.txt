idf_component_register(
    SRCS "main.c"
    PRIV_REQUIRES board_m5dial health settings control can_link aux_link obd ui_bridge wifi_link ota_client esp_driver_gpio app_update
    INCLUDE_DIRS "."
)
