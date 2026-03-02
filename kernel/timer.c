#include "timer.h"
#include "timebase.h"

status_t timer_init(const boot_info_t *boot_info) { return time_init(boot_info); }

BOOT_U64 timer_ticks(void) { return time_ticks(); }

BOOT_U64 timer_hz(void) { return time_hz(); }
