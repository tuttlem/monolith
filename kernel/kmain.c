#include "kernel.h"
#include "diag/boot_info.h"
#include "memory_init.h"
#include "page_alloc.h"

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
  boot_info_t *mutable_boot_info;

  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ") We good!\n");
  if (boot_info == (const boot_info_t *)0) {
    arch_puts("boot_info: null\n");
    goto spin;
  }

  mutable_boot_info = (boot_info_t *)boot_info;
  arch_memory_init(mutable_boot_info);
  page_alloc_init(mutable_boot_info);
  kprintf("Starting Monolith (%s) . . . \n", boot_info_arch_name(boot_info->arch_id));

  {
    page_alloc_stats_t alloc_stats;
    BOOT_U64 probe_page;

    page_alloc_stats(&alloc_stats);
    kprintf("page_alloc.available=%llu total_pages=%llu free_pages=%llu\n", alloc_stats.available,
            alloc_stats.total_pages, alloc_stats.free_pages);

    probe_page = alloc_page();
    if (probe_page != 0) {
      kprintf("page_alloc.probe.alloc=0x%llx\n", probe_page);
      free_page(probe_page);
      kprintf("page_alloc.probe.free=0x%llx\n", probe_page);
      page_alloc_stats(&alloc_stats);
      kprintf("page_alloc.after_probe free_pages=%llu\n", alloc_stats.free_pages);
    } else {
      kprintf("page_alloc.probe.unavailable\n");
    }
  }

#if MONOLITH_BOOTINFO_DEBUG
  diag_boot_info_print(boot_info);
#endif

spin:
  for (;;) {
    arch_halt();
  }
}
