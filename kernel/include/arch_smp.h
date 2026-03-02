#ifndef KERNEL_ARCH_SMP_H
#define KERNEL_ARCH_SMP_H

#include "kernel.h"

#define ARCH_SMP_API_VERSION_MAJOR 1U
#define ARCH_SMP_API_VERSION_MINOR 0U

/*
 * Returns:
 * - STATUS_OK when backend completed bootstrap actions
 * - STATUS_DEFERRED when backend is intentionally not active yet
 * - negative error on hard failure
 */
status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus);

#endif
