# duster-m5dial-controller

Firmware for an M5Dial (ESP32-S3) in-car controller and display for a Dacia Duster 2015 (Phase 1 facelift, X79): CAN vehicle data, engine DTC status, and aux light control of an AuxBeam RA60.

## Toolchain (pinned)

| Item | Version |
|---|---|
| ESP-IDF | **v5.5.5** (enforced by `idf: "==5.5.5"` in `main/idf_component.yml`) |
| LVGL | 9.5.0 (must match the EEZ Studio project setting) |
| M5Unified / M5GFX | 0.2.23 / 0.2.30 (board layer only, subject to the step 1 spike) |
| auxbeam-light-controller | **v0.1.1** git tag (private repo; the build machine needs GitHub access). Locked in `dependencies.lock` |

## Build

```
. ~/esp/esp-idf/export.sh
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

The host tests for the RA60 state machine (`aux_tests`) compile the library sources from a sibling checkout at `../auxbeam-light-controller`; check out tag `v0.1.1` there to match the firmware.

## Host tests

Pure logic (control core, button handling) is tested off-target:

```
cmake -S test/host -B build-host && cmake --build build-host && ./build-host/host_tests
```

## EEZ UI

`components/ui/` is EEZ Studio output (project `../duster-m5dial-ui`) and is never edited by hand. `ui_bridge` compiles it; run `idf.py reconfigure` after an export that adds or removes files.
