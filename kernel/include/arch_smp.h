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
status_t arch_smp_bootstrap(const boot_info_t *boot_info, u64 *out_possible_cpus, u64 *out_started_cpus);
status_t arch_smp_cpu_start(u64 cpu_id);
status_t arch_smp_ipi_send(u64 cpu_id, u64 kind);
status_t arch_smp_tlb_shootdown(u64 mask, u64 va, u64 len);

#endif
