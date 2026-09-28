idf_component_register(
    SRCS "aux_link.c" "aux_sm.c"
    INCLUDE_DIRS "include"
    REQUIRES control auxbeam-light-controller
    PRIV_REQUIRES board_m5dial esp_driver_uart esp_timer
)
