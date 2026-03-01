#include "kernel.h"
#include "diag/boot_info.h"

#ifndef CORE_ARCH_NAME
#define CORE_ARCH_NAME "unknown"
#endif

#ifndef MONOLITH_BOOTINFO_DEBUG
#define MONOLITH_BOOTINFO_DEBUG 1
#endif

static const char *boot_info_arch_name(BOOT_U64 arch_id) {
  switch (arch_id) {
  case BOOT_INFO_ARCH_X86_64:
    return "x86_64";
  case BOOT_INFO_ARCH_ARM64:
    return "arm64";
  case BOOT_INFO_ARCH_RISCV64:
    return "riscv64";
  case BOOT_INFO_ARCH_MIPS:
    return "mips";
  case BOOT_INFO_ARCH_SPARC64:
    return "sparc64";
  default:
    return "unknown";
  }
}


void kmain(const boot_info_t *boot_info) {
  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ") We good!\n");
  if (boot_info == (const boot_info_t *)0) {
    arch_puts("boot_info: null\n");
  } else {
    kprintf("Starting Monolith (%s) . . . \n", boot_info_arch_name(boot_info->arch_id));

#if MONOLITH_BOOTINFO_DEBUG
    diag_boot_info_print(boot_info);
#endif
  }

  for (;;) {
    arch_halt();
  }
}
