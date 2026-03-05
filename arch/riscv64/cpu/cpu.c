#include "arch_cpu.h"

static u64 g_riscv_cpu_id = 0;
static u64 g_riscv_initialized = 0;

status_t arch_cpu_early_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }
  if (g_riscv_initialized != 0) {
    return STATUS_OK;
  }

  g_riscv_cpu_id = boot_info->boot_cpu_id;
  g_riscv_initialized = 1;
  return STATUS_OK;
}

status_t arch_cpu_late_init(void) { return STATUS_OK; }

u64 arch_cpu_id(void) { return g_riscv_cpu_id; }

u64 arch_cpu_count_hint(void) { return 1ULL; }

void arch_cpu_relax(void) { __asm__ volatile("fence rw, rw" : : : "memory"); }

void arch_cpu_halt(void) { __asm__ volatile("wfi" : : : "memory"); }

void arch_cpu_reboot(void) {
  for (;;) {
    arch_cpu_halt();
  }
}

u64 arch_cycle_counter(void) {
  u64 v = 0;
  __asm__ volatile("csrr %0, cycle" : "=r"(v));
  return v;
}

status_t arch_cpu_set_local_base(u64 base) {
  __asm__ volatile("mv tp, %0" : : "r"(base) : "memory");
  return STATUS_OK;
}

u64 arch_cpu_get_local_base(void) {
  u64 base = 0;
  __asm__ volatile("mv %0, tp" : "=r"(base));
  return base;
}

void arch_barrier_full(void) { __asm__ volatile("fence rw, rw" : : : "memory"); }

void arch_barrier_read(void) { __asm__ volatile("fence r, r" : : : "memory"); }

void arch_barrier_write(void) { __asm__ volatile("fence w, w" : : : "memory"); }

void arch_tlb_sync_local(void) { __asm__ volatile("sfence.vma x0, x0" : : : "memory"); }

void arch_icache_sync_range(u64 addr, u64 size) {
  (void)addr;
  (void)size;
  __asm__ volatile("fence.i" : : : "memory");
}
