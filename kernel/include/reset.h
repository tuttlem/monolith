#ifndef KERNEL_RESET_H
#define KERNEL_RESET_H

#include "kernel.h"

typedef BOOT_U64 reset_id_t;

status_t reset_assert(reset_id_t rst);
status_t reset_deassert(reset_id_t rst);
status_t reset_pulse(reset_id_t rst);

#endif
