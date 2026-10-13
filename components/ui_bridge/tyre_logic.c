#include "tyre_logic.h"

int tyre_psi(uint8_t raw)
{
    return ((int)raw * 4351 + 5000) / 10000;
}

tyre_severity_t tyre_pressure_severity(uint8_t raw, bool stale,
                                        int caution_under_psi, int alert_under_psi,
                                        int caution_over_psi, int alert_over_psi)
{
    if (stale || raw == 0xFF) {
        return TYRE_SEV_NODATA;
    }
    int psi = tyre_psi(raw);
    if (psi < alert_under_psi || psi > alert_over_psi) {
        return TYRE_SEV_ALERT;
    }
    if (psi < caution_under_psi || psi > caution_over_psi) {
        return TYRE_SEV_CAUTION;
    }
    return TYRE_SEV_OK;
}

bool tyre_warning_condition(const tyre_pressures_t *tp, const tyre_psi_cfg_t *cfg)
{
    for (int pos = 0; pos < TYRE_COUNT; pos++) {
        bool front = (pos == TYRE_FL || pos == TYRE_FR);
        const tyre_psi_axle_cfg_t *axle = front ? &cfg->front : &cfg->rear;
        tyre_severity_t sev = tyre_pressure_severity(tp->raw[pos], tp->stale,
                                                      axle->caution_under_psi, axle->alert_under_psi,
                                                      axle->caution_over_psi, axle->alert_over_psi);
        if (sev == TYRE_SEV_NODATA) {
            continue;  // stale or no value never raises it (section 9)
        }
        if (sev == TYRE_SEV_CAUTION || sev == TYRE_SEV_ALERT) {
            return true;
        }
        if (tp->state[pos] == 5 || tp->state[pos] == 6) {
            return true;
        }
    }
    if (!tp->stale && tp->failure) {
        return true;
    }
    return false;
}

bool tyre_warn_update(tyre_warn_state_t *st, bool condition_now, uint32_t now_ms, uint32_t set_ms, uint32_t clear_ms)
{
    if (condition_now != st->condition_prev) {
        st->condition_prev = condition_now;
        st->transition_ms = now_ms;
    }
    uint32_t held_ms = (uint32_t)(now_ms - st->transition_ms);
    if (condition_now && !st->raised && held_ms >= set_ms) {
        st->raised = true;
    } else if (!condition_now && st->raised && held_ms >= clear_ms) {
        st->raised = false;
    }
    return st->raised;
}

bool wheel_slip_gate_open(const wheel_speeds_t *ws, uint16_t speed_gate_centi_kmh)
{
    return !ws->stale && ws->speed_centi_kmh >= speed_gate_centi_kmh;
}

uint16_t wheel_speed_median(const uint16_t raw[TYRE_COUNT])
{
    uint16_t v[TYRE_COUNT];
    for (int i = 0; i < TYRE_COUNT; i++) {
        v[i] = raw[i];
    }
    for (int i = 1; i < TYRE_COUNT; i++) {
        uint16_t key = v[i];
        int j = i - 1;
        while (j >= 0 && v[j] > key) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = key;
    }
    return (uint16_t)(((uint32_t)v[1] + v[2]) / 2);
}

int wheel_slip_pct(uint16_t raw, uint16_t reference)
{
    if (reference == 0) {
        return 0;
    }
    int32_t pct = ((int32_t)raw - (int32_t)reference) * 100 / (int32_t)reference;
    if (pct > 99) {
        return 99;
    }
    if (pct < -99) {
        return -99;
    }
    return (int)pct;
}

float wheel_slip_average_update(float prev_avg, int slip_pct_now, uint32_t dt_ms, uint32_t tau_ms)
{
    float alpha = (float)dt_ms / (float)(tau_ms + dt_ms);
    return prev_avg + alpha * ((float)slip_pct_now - prev_avg);
}

tyre_severity_t wheel_slip_severity(tyre_severity_t prev, bool nodata, int slip_pct_abs,
                                     int caution_pct, int alert_pct, int hysteresis_pct)
{
    if (nodata) {
        return TYRE_SEV_NODATA;
    }
    if (slip_pct_abs >= alert_pct) {
        return TYRE_SEV_ALERT;
    }
    if (prev == TYRE_SEV_ALERT && slip_pct_abs >= alert_pct - hysteresis_pct) {
        return TYRE_SEV_ALERT;
    }
    if (slip_pct_abs >= caution_pct) {
        return TYRE_SEV_CAUTION;
    }
    if ((prev == TYRE_SEV_CAUTION || prev == TYRE_SEV_ALERT) && slip_pct_abs >= caution_pct - hysteresis_pct) {
        return TYRE_SEV_CAUTION;
    }
    return TYRE_SEV_OK;
}
