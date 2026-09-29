idf_component_register(
    SRCS "can_link.c" "can_signals.c"
    INCLUDE_DIRS "include"
    REQUIRES control
    PRIV_REQUIRES board_m5dial health esp_driver_twai esp_timer
)
