#include "engine_logic.h"

tyre_severity_t coolant_severity(int16_t temp_c, bool stale,
                                  int caution_under_c, int alert_under_c,
                                  int caution_over_c, int alert_over_c)
{
    if (stale) {
        return TYRE_SEV_NODATA;
    }
    if (temp_c < alert_under_c || temp_c > alert_over_c) {
        return TYRE_SEV_ALERT;
    }
    if (temp_c < caution_under_c || temp_c > caution_over_c) {
        return TYRE_SEV_CAUTION;
    }
    return TYRE_SEV_OK;
}
