// PHASE 2 ONLY (ESP-NOW remote display). Not used in Phase 1.
// Copy to secrets.h (git-ignored) and fill in when Phase 2 starts.
#pragma once

// STA MAC addresses
#define DUSTER_CONTROLLER_MAC {0x00,0x00,0x00,0x00,0x00,0x00}
#define DUSTER_REMOTE_MAC     {0x00,0x00,0x00,0x00,0x00,0x00}

// ESP-NOW keys, 16 bytes each. PMK must be set explicitly. LMK unique to this pair.
#define DUSTER_ESPNOW_PMK "0000000000000000"
#define DUSTER_ESPNOW_LMK "0000000000000000"
