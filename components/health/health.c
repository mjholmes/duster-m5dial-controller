#include "health.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "health";

void health_task_start(void)
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
}

void health_task_alive(void)
{
#ifdef CONFIG_DIAG_HANG_TASK
    // Fault injection: block the named task forever so it stops feeding the
    // watchdog; the watchdog must reset the device into MANUAL, lights off.
    if (CONFIG_DIAG_HANG_TASK[0] != '\0' && esp_timer_get_time() > 10 * 1000000LL &&
        strcmp(pcTaskGetName(NULL), CONFIG_DIAG_HANG_TASK) == 0) {
        ESP_LOGW(TAG, "fault injection: hanging task %s", CONFIG_DIAG_HANG_TASK);
        vTaskDelay(portMAX_DELAY);
    }
#endif
    esp_task_wdt_reset();
}

void health_log_reset_reason(void)
{
    esp_reset_reason_t r = esp_reset_reason();
    const char *name;
    switch (r) {
    case ESP_RST_POWERON:  name = "power-on"; break;
    case ESP_RST_SW:       name = "software"; break;
    case ESP_RST_PANIC:    name = "panic"; break;
    case ESP_RST_INT_WDT:  name = "interrupt watchdog"; break;
    case ESP_RST_TASK_WDT: name = "task watchdog"; break;
    case ESP_RST_WDT:      name = "other watchdog"; break;
    case ESP_RST_BROWNOUT: name = "brown-out"; break;
    case ESP_RST_USB:      name = "USB"; break;
    default:               name = "other"; break;
    }
    if (r == ESP_RST_POWERON || r == ESP_RST_USB || r == ESP_RST_SW) {
        ESP_LOGI(TAG, "reset reason: %s", name);
    } else {
        // Abnormal reset: the device restarts in MANUAL with lights off.
        ESP_LOGW(TAG, "reset reason: %s (%d)", name, r);
    }
}
