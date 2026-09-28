#include "brightness.h"

int brightness_step(int pct, int32_t detents, const brightness_cfg_t *cfg)
{
    int64_t next = (int64_t)pct + (int64_t)detents * cfg->step_pct;
    if (next < cfg->min_pct) {
        next = cfg->min_pct;
    }
    if (next > cfg->max_pct) {
        next = cfg->max_pct;
    }
    return (int)next;
}

uint8_t brightness_to_level(int pct)
{
    if (pct <= 0) {
        return 0;
    }
    if (pct >= 100) {
        return 255;
    }
    return (uint8_t)((pct * 255 + 50) / 100);
}

int brightness_effective(int pct, bool night, int dim_pct, int min_pct)
{
    if (!night) {
        return pct;
    }
    int dimmed = pct - dim_pct;
    return dimmed < min_pct ? min_pct : dimmed;
}
