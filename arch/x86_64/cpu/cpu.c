#include "arch_cpu.h"

static BOOT_U64 g_x86_cpu_id = 0;
static BOOT_U64 g_x86_initialized = 0;

static void x86_cpuid(BOOT_U32 leaf, BOOT_U32 *eax, BOOT_U32 *ebx, BOOT_U32 *ecx, BOOT_U32 *edx) {
  BOOT_U32 a, b, c, d;
  __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(leaf));
  if (eax != (BOOT_U32 *)0) {
    *eax = a;
  }
  if (ebx != (BOOT_U32 *)0) {
    *ebx = b;
  }
  if (ecx != (BOOT_U32 *)0) {
    *ecx = c;
  }
  if (edx != (BOOT_U32 *)0) {
    *edx = d;
  }
}

status_t arch_cpu_early_init(const boot_info_t *boot_info) {
  BOOT_U32 ebx = 0;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }
  if (g_x86_initialized != 0) {
    return STATUS_OK;
  }

  x86_cpuid(1U, (BOOT_U32 *)0, &ebx, (BOOT_U32 *)0, (BOOT_U32 *)0);
  g_x86_cpu_id = (BOOT_U64)((ebx >> 24) & 0xFFU);
  g_x86_initialized = 1;
  return STATUS_OK;
}

status_t arch_cpu_late_init(void) { return STATUS_OK; }

BOOT_U64 arch_cpu_id(void) { return g_x86_cpu_id; }

BOOT_U64 arch_cpu_count_hint(void) { return 1ULL; }

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

BOOT_U64 arch_cycle_counter(void) {
  BOOT_U32 lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((BOOT_U64)hi << 32) | (BOOT_U64)lo;
}

void arch_barrier_full(void) { __asm__ volatile("mfence" : : : "memory"); }

void arch_barrier_read(void) { __asm__ volatile("lfence" : : : "memory"); }

void arch_barrier_write(void) { __asm__ volatile("sfence" : : : "memory"); }

void arch_tlb_sync_local(void) {
  BOOT_U64 cr3;
  __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
  __asm__ volatile("movq %0, %%cr3" : : "r"(cr3) : "memory");
}

void arch_icache_sync_range(BOOT_U64 addr, BOOT_U64 size) {
  (void)addr;
  (void)size;
  /* x86 i-cache is coherent for normal code writes in this bring-up model. */
  __asm__ volatile("" : : : "memory");
}
