#include "kernel.h"
#include "diag/boot_info.h"
#include "interrupts.h"
#include "kmalloc.h"
#include "memory_init.h"
#include "page_alloc.h"
#include "timer.h"

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

#ifndef MONOLITH_TIMER_SELFTEST
#define MONOLITH_TIMER_SELFTEST 0
#endif

#ifndef MONOLITH_TIMER_SELFTEST_SPINS
#define MONOLITH_TIMER_SELFTEST_SPINS 200000000ULL
#endif

#ifndef MONOLITH_EXCEPTION_SELFTEST
#define MONOLITH_EXCEPTION_SELFTEST 0
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

#if MONOLITH_TIMER_SELFTEST
static BOOT_U64 timer_selftest_spins_for_arch(BOOT_U64 arch_id) {
  if (arch_id == BOOT_INFO_ARCH_X86_64) {
    return MONOLITH_TIMER_SELFTEST_SPINS;
  }
  return 20000000ULL;
}

static int timer_progress_selftest(BOOT_U64 spins) {
  BOOT_U64 start = timer_ticks();
  BOOT_U64 last = start;
  BOOT_U64 i;

  for (i = 0; i < spins; ++i) {
    last = timer_ticks();
    if (last != start) {
      return 1;
    }
    __asm__ volatile("" : : : "memory");
  }
  return 0;
}
#endif


void kmain(const boot_info_t *boot_info) {
  boot_info_t *mutable_boot_info;
  status_t mem_status;
  status_t page_status;
  status_t heap_status;
  status_t irq_status;
  status_t timer_status;

  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ") We good!\n");
  if (boot_info == (const boot_info_t *)0) {
    arch_puts("boot_info: null\n");
    goto spin;
  }

  mutable_boot_info = (boot_info_t *)boot_info;
  mem_status = arch_memory_init(mutable_boot_info);
  page_status = page_alloc_init(mutable_boot_info);
  heap_status = kmalloc_init(mutable_boot_info);
  irq_status = interrupts_init(boot_info);
  timer_status = timer_init(boot_info);

  if (!status_is_ok(mem_status) && mem_status != STATUS_DEFERRED) {
    kprintf("arch_memory_init: %s (%d)\n", status_str(mem_status), mem_status);
  }
  if (!status_is_ok(page_status) && page_status != STATUS_DEFERRED) {
    kprintf("page_alloc_init: %s (%d)\n", status_str(page_status), page_status);
  }
  if (!status_is_ok(heap_status) && heap_status != STATUS_DEFERRED) {
    kprintf("kmalloc_init: %s (%d)\n", status_str(heap_status), heap_status);
  }
  if (!status_is_ok(irq_status) && irq_status != STATUS_DEFERRED) {
    kprintf("interrupts_init: %s (%d)\n", status_str(irq_status), irq_status);
  }
  if (!status_is_ok(timer_status) && timer_status != STATUS_DEFERRED) {
    kprintf("timer_init: %s (%d)\n", status_str(timer_status), timer_status);
  }

#if MONOLITH_TIMER_SELFTEST
  if (timer_status == STATUS_OK) {
    BOOT_U64 spins = timer_selftest_spins_for_arch(boot_info->arch_id);
    if (timer_progress_selftest(spins)) {
      kprintf("timer self-test: PASS (hz=%llu ticks=%llu)\n", timer_hz(), timer_ticks());
    } else {
      kprintf("timer self-test: FAILED (ticks did not advance)\n");
    }
  } else if (timer_status == STATUS_DEFERRED) {
    kprintf("timer self-test: SKIP (deferred)\n");
  }
#endif

#if MONOLITH_EXCEPTION_SELFTEST
  kprintf("exception self-test: triggering deliberate exception\n");
  arch_exception_selftest_trigger();
  kprintf("exception self-test: unexpected return from trigger\n");
#endif

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
