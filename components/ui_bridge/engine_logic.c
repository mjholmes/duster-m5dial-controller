#include "engine_logic.h"

coolant_color_t coolant_color_class(int16_t temp_c, bool stale, int cold_under_c, int hot_over_c)
{
    if (stale) {
        return COOLANT_COLOR_NORMAL;
    }
    if (temp_c < cold_under_c) {
        return COOLANT_COLOR_COLD;
    }
    if (temp_c > hot_over_c) {
        return COOLANT_COLOR_HOT;
    }
    return COOLANT_COLOR_NORMAL;
}
