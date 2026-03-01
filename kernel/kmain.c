#include "kernel.h"
#include "diag/boot_info.h"
#include "kmalloc.h"
#include "memory_init.h"
#include "page_alloc.h"

#ifndef CORE_ARCH_NAME
#define CORE_ARCH_NAME "unknown"
#endif

#ifndef MONOLITH_BOOTINFO_DEBUG
#define MONOLITH_BOOTINFO_DEBUG 1
#endif

#ifndef MONOLITH_KMALLOC_SELFTEST
#define MONOLITH_KMALLOC_SELFTEST 1
#endif

#ifndef MONOLITH_KMALLOC_DEBUG_EXERCISE
#define MONOLITH_KMALLOC_DEBUG_EXERCISE 1
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
  kmalloc_init(mutable_boot_info);

#if MONOLITH_KMALLOC_SELFTEST
  if (!kmalloc_self_test()) {
    kprintf("kmalloc self-test: FAILED\n");
  }
#endif

#if MONOLITH_KMALLOC_DEBUG_EXERCISE
  {
    void *ptrs[8];
    kmalloc_stats_t st;
    BOOT_U32 i;

    kmalloc_stats(&st);
    kprintf("kmalloc dbg: begin used=%llu free=%llu allocs=%llu frees=%llu\n", st.bytes_used, st.bytes_free,
            st.alloc_count, st.free_count);

    for (i = 0; i < 8U; ++i) {
      ptrs[i] = kmalloc(64 + (BOOT_U64)i * 16ULL);
      kprintf("kmalloc dbg: alloc[%u]=%p\n", i, ptrs[i]);
    }
    kmalloc_stats(&st);
    kprintf("kmalloc dbg: after alloc used=%llu free=%llu allocs=%llu frees=%llu\n", st.bytes_used, st.bytes_free,
            st.alloc_count, st.free_count);

    for (i = 0; i < 4U; ++i) {
      kfree(ptrs[i]);
      kprintf("kmalloc dbg: free[%u]=%p\n", i, ptrs[i]);
    }
    kmalloc_stats(&st);
    kprintf("kmalloc dbg: after free4 used=%llu free=%llu allocs=%llu frees=%llu\n", st.bytes_used, st.bytes_free,
            st.alloc_count, st.free_count);

    for (i = 4U; i < 8U; ++i) {
      kfree(ptrs[i]);
      kprintf("kmalloc dbg: free[%u]=%p\n", i, ptrs[i]);
    }
    kmalloc_stats(&st);
    kprintf("kmalloc dbg: after free8 used=%llu free=%llu allocs=%llu frees=%llu invalid=%llu double=%llu\n",
            st.bytes_used, st.bytes_free, st.alloc_count, st.free_count, st.invalid_free_count,
            st.double_free_count);
  }
#endif

  kprintf("Starting Monolith (%s) . . . \n", boot_info_arch_name(boot_info->arch_id));

#if MONOLITH_BOOTINFO_DEBUG
  diag_boot_info_print(boot_info);
#endif

spin:
  for (;;) {
    arch_halt();
  }
}
