idf_component_register(
    SRCS "isotp_rx.c" "obd_decode.c" "obd_core.c"
    INCLUDE_DIRS "include"
    REQUIRES control
)
