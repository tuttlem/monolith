#ifndef KERNEL_SMP_H
#define KERNEL_SMP_H

#include "kernel.h"
#include "ipi.h"

#define SMP_API_VERSION_MAJOR 1U
#define SMP_API_VERSION_MINOR 0U

status_t smp_init(const boot_info_t *boot_info);
status_t smp_cpu_start(BOOT_U64 cpu_id);
BOOT_U64 smp_cpu_count_online(void);
BOOT_U64 smp_cpu_count_possible(void);

/*
 * Common secondary CPU C entry for architecture backends.
 * Current phase policy: publish online and park in idle loop.
 */
void smp_secondary_entry(BOOT_U64 cpu_id);

#endif
