cmake_minimum_required(VERSION 3.16)

# CI sets FIRMWARE_VERSION from the pushed tag with the leading "v" stripped
# (docs/ota-update-spec.md 4.3), so the running version string matches the
# OTA manifest's byte for byte. Local/dev builds fall through to the
# existing git-describe-based default.
if(DEFINED ENV{FIRMWARE_VERSION})
    set(PROJECT_VER $ENV{FIRMWARE_VERSION})
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(duster-m5dial-controller)
