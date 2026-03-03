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
status_t arch_smp_cpu_start(BOOT_U64 cpu_id);
status_t arch_smp_ipi_send(BOOT_U64 cpu_id, BOOT_U64 kind);
status_t arch_smp_tlb_shootdown(BOOT_U64 mask, BOOT_U64 va, BOOT_U64 len);

#endif
