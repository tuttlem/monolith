#include "arch_cpu.h"

static u64 g_x86_cpu_id = 0;
static u64 g_x86_initialized = 0;

#define X86_64_MSR_GS_BASE 0xC0000101U

static void x86_cpuid(u32 leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
  u32 a, b, c, d;
  __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(leaf));
  if (eax != (u32 *)0) {
    *eax = a;
  }
  if (ebx != (u32 *)0) {
    *ebx = b;
  }
  if (ecx != (u32 *)0) {
    *ecx = c;
  }
  if (edx != (u32 *)0) {
    *edx = d;
  }
}

static void x86_write_msr(u32 msr, u64 value) {
  u32 lo = (u32)(value & 0xFFFFFFFFULL);
  u32 hi = (u32)(value >> 32);
  __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi) : "memory");
}

static u64 x86_read_msr(u32 msr) {
  u32 lo;
  u32 hi;
  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return ((u64)hi << 32) | (u64)lo;
}

status_t arch_cpu_early_init(const boot_info_t *boot_info) {
  u32 ebx = 0;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }
  if (g_x86_initialized != 0) {
    return STATUS_OK;
  }

  x86_cpuid(1U, (u32 *)0, &ebx, (u32 *)0, (u32 *)0);
  g_x86_cpu_id = (u64)((ebx >> 24) & 0xFFU);
  g_x86_initialized = 1;
  return STATUS_OK;
}

status_t arch_cpu_late_init(void) { return STATUS_OK; }

u64 arch_cpu_id(void) { return g_x86_cpu_id; }

u64 arch_cpu_count_hint(void) { return 1ULL; }

void arch_cpu_relax(void) { __asm__ volatile("pause" : : : "memory"); }

void arch_cpu_halt(void) { __asm__ volatile("hlt" : : : "memory"); }

static void x86_outb(unsigned short port, unsigned char value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void arch_cpu_reboot(void) {
  x86_outb(0x64U, 0xFEU);
  for (;;) {
    arch_cpu_halt();
  }
}

u64 arch_cycle_counter(void) {
  u32 lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((u64)hi << 32) | (u64)lo;
}

status_t arch_cpu_set_local_base(u64 base) {
  x86_write_msr(X86_64_MSR_GS_BASE, base);
  return STATUS_OK;
}

u64 arch_cpu_get_local_base(void) { return x86_read_msr(X86_64_MSR_GS_BASE); }

void arch_barrier_full(void) { __asm__ volatile("mfence" : : : "memory"); }

void arch_barrier_read(void) { __asm__ volatile("lfence" : : : "memory"); }

void arch_barrier_write(void) { __asm__ volatile("sfence" : : : "memory"); }

void arch_tlb_sync_local(void) {
  u64 cr3;
  __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
  __asm__ volatile("movq %0, %%cr3" : : "r"(cr3) : "memory");
}

void arch_icache_sync_range(u64 addr, u64 size) {
  (void)addr;
  (void)size;
  /* x86 i-cache is coherent for normal code writes in this bring-up model. */
  __asm__ volatile("" : : : "memory");
}
