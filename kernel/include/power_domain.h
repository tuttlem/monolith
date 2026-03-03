#ifndef KERNEL_POWER_DOMAIN_H
#define KERNEL_POWER_DOMAIN_H

#include "kernel.h"

typedef BOOT_U64 power_domain_id_t;

status_t power_domain_on(power_domain_id_t pd);
status_t power_domain_off(power_domain_id_t pd);
status_t power_domain_status(power_domain_id_t pd, BOOT_U64 *out_on);

#endif
