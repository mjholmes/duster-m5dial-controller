# Host (off-target) tests for pure logic. Not part of the firmware build.
#   cmake -S test/host -B build-host && cmake --build build-host && ./build-host/host_tests
cmake_minimum_required(VERSION 3.16)
project(duster_host_tests C)

set(ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_executable(host_tests
    test_main.c
    ${ROOT}/components/control/control_core.c
    ${ROOT}/components/ui_bridge/button_fsm.c
    ${ROOT}/components/ui_bridge/brightness.c
    ${ROOT}/components/can_link/can_signals.c
)
target_include_directories(host_tests PRIVATE
    ${ROOT}/components/control/include
    ${ROOT}/components/ui_bridge
    ${ROOT}/components/can_link/include
)
target_compile_options(host_tests PRIVATE -std=c11 -Wall -Wextra -Werror)

# RA60 send state machine against the real auxbeam-light-controller sources
# and the library's mock UART / mock clock (sibling checkout).
set(AUXBEAM ${ROOT}/../auxbeam-light-controller)
add_executable(aux_tests
    test_aux.c
    ${ROOT}/components/aux_link/aux_sm.c
    ${AUXBEAM}/src/checksum.c
    ${AUXBEAM}/src/light_protocol.c
    ${AUXBEAM}/src/command_queue.c
    ${AUXBEAM}/src/auxbeam_light.c
    ${AUXBEAM}/test/host/mock_hal_uart.c
)
target_include_directories(aux_tests PRIVATE
    ${ROOT}/components/aux_link/include
    ${ROOT}/components/control/include
    ${AUXBEAM}/include
    ${AUXBEAM}/src
    ${AUXBEAM}/test/host
    ${AUXBEAM}/test/host/stubs
)
target_compile_options(aux_tests PRIVATE -std=c11 -Wall -Wextra)
