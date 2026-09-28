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
)
target_include_directories(host_tests PRIVATE
    ${ROOT}/components/control/include
    ${ROOT}/components/ui_bridge
)
target_compile_options(host_tests PRIVATE -std=c11 -Wall -Wextra -Werror)
