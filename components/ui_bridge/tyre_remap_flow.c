#include "tyre_remap_flow.h"

static void remaining_candidates(const tyre_remap_t *r, uint8_t *out, int *out_n)
{
    int n = 0;
    for (int c = 0; c < 4; c++) {
        bool used = false;
        for (int i = 0; i < r->step; i++) {
            if (r->assigned[i] == (uint8_t)c) {
                used = true;
                break;
            }
        }
        if (!used) {
            out[n++] = (uint8_t)c;
        }
    }
    *out_n = n;
}

void tyre_remap_start(tyre_remap_t *r)
{
    r->step = 0;
    r->candidate_idx = 0;
    for (int i = 0; i < 4; i++) {
        r->assigned[i] = 0;
    }
}

void tyre_remap_rotate(tyre_remap_t *r, int dir)
{
    uint8_t cand[4];
    int n;
    remaining_candidates(r, cand, &n);
    if (n < 2) {
        return;
    }
    r->candidate_idx = ((r->candidate_idx + dir) % n + n) % n;
}

uint8_t tyre_remap_candidate(const tyre_remap_t *r)
{
    uint8_t cand[4];
    int n;
    remaining_candidates(r, cand, &n);
    if (n == 0) {
        return 0;
    }
    int idx = r->candidate_idx;
    if (idx < 0 || idx >= n) {
        idx = 0;
    }
    return cand[idx];
}

bool tyre_remap_confirm(tyre_remap_t *r, uint8_t out_map[4])
{
    // Defensive: if this is ever called again after the flow already
    // completed (e.g. a queued tap delivered just as the screen changed),
    // r->step == 4 and writing assigned[4] would be out of bounds. Treat
    // it as already-done rather than corrupting state.
    if (r->step >= 4) {
        for (int i = 0; i < 4; i++) {
            out_map[i] = r->assigned[i];
        }
        return true;
    }
    r->assigned[r->step] = tyre_remap_candidate(r);
    r->step++;
    r->candidate_idx = 0;
    if (r->step >= 4) {
        for (int i = 0; i < 4; i++) {
            out_map[i] = r->assigned[i];
        }
        return true;
    }
    return false;
}
