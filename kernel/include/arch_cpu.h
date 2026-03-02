#ifndef KERNEL_ARCH_CPU_H
#define KERNEL_ARCH_CPU_H

#include "kernel.h"

#define ARCH_CPU_API_VERSION_MAJOR 1U
#define ARCH_CPU_API_VERSION_MINOR 0U

/*
 * Stable CPU HAL interface.
 * Contract version 1.0 is frozen by spec 040.
 */
status_t arch_cpu_early_init(const boot_info_t *boot_info);
status_t arch_cpu_late_init(void);

BOOT_U64 arch_cpu_id(void);
BOOT_U64 arch_cpu_count_hint(void);

void arch_cpu_relax(void);
void arch_cpu_halt(void);
void arch_cpu_reboot(void);

BOOT_U64 arch_cycle_counter(void);

status_t arch_cpu_set_local_base(BOOT_U64 base);
BOOT_U64 arch_cpu_get_local_base(void);

void arch_barrier_full(void);
void arch_barrier_read(void);
void arch_barrier_write(void);

void arch_tlb_sync_local(void);
void arch_icache_sync_range(BOOT_U64 addr, BOOT_U64 size);

#endif
