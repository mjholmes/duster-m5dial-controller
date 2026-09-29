idf_component_register(
    SRCS "isotp_rx.c" "obd_decode.c" "obd_core.c" "obd_client.c"
    INCLUDE_DIRS "include"
    REQUIRES control
    PRIV_REQUIRES can_link health esp_timer
)
