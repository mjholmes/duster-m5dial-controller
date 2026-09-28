idf_component_register(
    SRCS "control.c" "control_core.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_timer
)
