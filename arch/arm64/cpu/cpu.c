#include "arch_cpu.h"

static BOOT_U64 g_arm64_cpu_id = 0;
static BOOT_U64 g_arm64_initialized = 0;

status_t arch_cpu_early_init(const boot_info_t *boot_info) {
  BOOT_U64 mpidr = 0;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }
  if (g_arm64_initialized != 0) {
    return STATUS_OK;
  }

  __asm__ volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
  g_arm64_cpu_id = mpidr & 0xFFFFFFULL;
  g_arm64_initialized = 1;
  return STATUS_OK;
}

status_t arch_cpu_late_init(void) { return STATUS_OK; }

BOOT_U64 arch_cpu_id(void) { return g_arm64_cpu_id; }

BOOT_U64 arch_cpu_count_hint(void) { return 1ULL; }

void arch_cpu_relax(void) { __asm__ volatile("yield" : : : "memory"); }

void arch_cpu_halt(void) { __asm__ volatile("wfi" : : : "memory"); }

void arch_cpu_reboot(void) {
  for (;;) {
    arch_cpu_halt();
  }
}

BOOT_U64 arch_cycle_counter(void) {
  BOOT_U64 v = 0;
  __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
  return v;
}

void arch_barrier_full(void) { __asm__ volatile("dsb sy" : : : "memory"); }

void arch_barrier_read(void) { __asm__ volatile("dmb ishld" : : : "memory"); }

void arch_barrier_write(void) { __asm__ volatile("dmb ishst" : : : "memory"); }

void arch_tlb_sync_local(void) {
  __asm__ volatile("dsb ish\n\t"
                   "tlbi vmalle1\n\t"
                   "dsb ish\n\t"
                   "isb\n\t"
                   :
                   :
                   : "memory");
}

void arch_icache_sync_range(BOOT_U64 addr, BOOT_U64 size) {
  (void)addr;
  (void)size;
  __asm__ volatile("ic iallu\n\t"
                   "dsb ish\n\t"
                   "isb\n\t"
                   :
                   :
                   : "memory");
}
