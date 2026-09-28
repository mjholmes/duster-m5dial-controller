idf_component_register(
    SRCS "main.c"
    PRIV_REQUIRES board_m5dial settings control can_link ui_bridge esp_driver_gpio
    INCLUDE_DIRS "."
)
