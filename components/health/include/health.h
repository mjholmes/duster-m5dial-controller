// Task watchdog membership and fault injection (SPEC 5 Watchdog, 11 step 7).
#pragma once

// Subscribe the calling task to the task watchdog (3 s, panic and reset).
void health_task_start(void);

// Feed the watchdog. Call once per loop; every task loop waits for a bounded
// time (at most CONFIG_QUEUE_WAIT_MS), so a healthy task always feeds in time.
// With CONFIG_DIAG_HANG_TASK set, the named task hangs here 10 s after boot.
void health_task_alive(void);

// Log the reset reason; call first thing in app_main.
void health_log_reset_reason(void);
