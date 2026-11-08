// User preferences kept in NVS across power cycles (display settings and
// the OTA channel choice; mode and light state are never persisted, SPEC 6.2).
//
// Writes are deferred: a change is committed only once it has been stable for
// CONFIG_SETTINGS_SAVE_DELAY_MS, so turning the encoder does not write flash
// on every detent. An abrupt power-off before that loses only the last change.
//
// Threading: ui task only in Phase 1.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Initialise NVS and load settings (defaults if absent or invalid).
esp_err_t settings_init(void);

// Display brightness for day (night = false) or night (night = true).
int settings_brightness_pct(bool night);
void settings_set_brightness_pct(bool night, int pct);

// OTA release channel (docs/ota-update-spec.md 7, R2). Default stable.
// Committed immediately -- unlike brightness, this changes rarely enough
// that flash wear from debouncing isn't a concern.
bool settings_ota_beta_channel(void);
void settings_set_ota_beta_channel(bool beta);

// Tyre position remap (issue #1): for physical corner i (0=FL,1=FR,2=RL,
//3=RR, matching tyre_pos_t), which CAN-reported corner's reading to show
// there. Just storage -- settings has no notion of "valid permutation";
// the caller (ui_vars, via tyre_logic.c's tyre_map_is_valid()) validates
// and falls back to identity if what's stored isn't one. Defaults to
// identity {0,1,2,3} until first set. Committed immediately, same
// reasoning as the OTA channel: changes rarely enough that flash wear
// from debouncing isn't a concern.
void settings_tyre_map(uint8_t map[4]);
void settings_set_tyre_map(const uint8_t map[4]);

// Commit pending changes once they have been stable long enough. Call each loop.
void settings_service(uint32_t now_ms);
