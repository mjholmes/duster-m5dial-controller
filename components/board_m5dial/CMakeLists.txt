idf_component_register(
    SRCS "board_m5dial.cpp" "encoder.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio
    PRIV_REQUIRES esp_driver_pcnt
)
