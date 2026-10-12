# components/ui is EEZ Studio output and is never edited, so it has no
# CMakeLists.txt of its own; its sources are compiled here. Run
# `idf.py reconfigure` after an export that adds or removes files.
set(EEZ_UI_DIR "${CMAKE_CURRENT_LIST_DIR}/../ui")
file(GLOB EEZ_UI_SRCS "${EEZ_UI_DIR}/*.c")

idf_component_register(
    SRCS "ui_bridge.c" "lvgl_port.c" "ui_vars.c" "ui_actions.c" "ui_styles.c" "button_fsm.c" "brightness.c" "tyre_logic.c" ${EEZ_UI_SRCS}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "${EEZ_UI_DIR}"
    REQUIRES control
    PRIV_REQUIRES board_m5dial settings health esp_timer esp_app_format wifi_link ota_client
)
