#include "kernel.h"
#include "arch_cpu.h"
#include "audio.h"
#include "config.h"
#include "capability_profile.h"
#include "device_bus.h"
#include "device_domains.h"
#include "device_model.h"
#include "diag/boot_info.h"
#include "hw_desc.h"
#include "hw_resource.h"
#include "irq_domain.h"
#include "interrupts.h"
#include "input.h"
#include "iommu.h"
#include "kmalloc.h"
#include "arch_mm.h"
#include "device_report.h"
#include "dma.h"
#include "net.h"
#include "panic.h"
#include "page_alloc.h"
#include "pci.h"
#include "percpu.h"
#include "personality.h"
#include "smp.h"
#include "syscall.h"
#include "scheduler.h"
#include "timebase.h"
#include "timer.h"
#include "trace.h"
#include "usb.h"
#include "uaccess.h"
#include "user_syscall.h"
#include "user_task.h"
#include "arch_input.h"
#include "arch_user_mode.h"

#ifndef CORE_ARCH_NAME
#define CORE_ARCH_NAME "unknown"
#endif

#if defined(__x86_64__) || defined(__aarch64__) || defined(__riscv)
static unsigned char g_usermode_kernel_stack[16384] __attribute__((aligned(16)));

__attribute__((noreturn)) static void usermode_probe_entry(void *arg) {
  volatile u64 sink = 0ULL;
  u64 ret = 0ULL;
  char msg[] = "usermode: probe trap ok";
  (void)arg;

  (void)user_syscall2(SYSCALL_OP_DEBUG_LOG, (u64)(uptr)msg, (u64)(sizeof(msg) - 1U), &ret);

  for (;;) {
    (void)user_syscall0(SYSCALL_OP_TIME_NOW, &ret);
    sink ^= ret;
    __asm__ volatile("" : : "r"(sink) : "memory");
  }
}

static status_t usermode_probe_launch(const boot_info_t *boot_info) {
  const u64 k_user_stack_bytes = 4096ULL;
  u64 page_size;
  u64 stack_base;
#if !defined(__riscv)
  u64 stack_page;
#endif
  u64 user_sp;
  u64 user_entry;
  u64 entry;
  u64 entry_page;
#if defined(__riscv)
  u64 user_region_base = 0ULL;
#endif
  u64 flags = 0ULL;
  mm_phys_addr_t pa = 0ULL;
  status_t st;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  page_size = mm_page_size();
  if (page_size == 0ULL) {
    return STATUS_FAULT;
  }
  entry = (u64)(uptr)usermode_probe_entry;
  entry_page = entry & ~(page_size - 1ULL);

  st = user_stack_alloc(page_size, &stack_base);
  if (st != STATUS_OK) {
    return st;
  }
#if !defined(__riscv)
  stack_page = stack_base & ~(page_size - 1ULL);
#endif

#if defined(__riscv)
  /*
   * riscv64 currently uses 1GiB leaf mappings in the substrate. Marking the
   * kernel's own 1GiB region as user can fault S-mode instruction fetches.
   * Alias only the entry page to a user VA and jump to the alias.
   */
  user_region_base = entry_page - page_size;
  st = mm_map(user_region_base, entry_page, page_size, MMU_PROT_READ | MMU_PROT_WRITE | MMU_PROT_EXEC | MMU_PROT_USER);
  if (st != STATUS_OK) {
    return st;
  }
  user_entry = user_region_base + (entry - entry_page);
  user_sp = user_region_base + (stack_base - entry_page) + k_user_stack_bytes - 16ULL;
#else
  st = mm_protect(entry_page, page_size, MMU_PROT_READ | MMU_PROT_WRITE | MMU_PROT_EXEC | MMU_PROT_USER);
  if (st != STATUS_OK) {
    return st;
  }
  if (stack_page != entry_page) {
    st = mm_protect(stack_page, page_size, MMU_PROT_READ | MMU_PROT_WRITE | MMU_PROT_USER);
    if (st != STATUS_OK) {
      return st;
    }
  }
  st = mm_sync_tlb(entry_page, page_size);
  if (st != STATUS_OK) {
    return st;
  }
  if (stack_page != entry_page) {
    st = mm_sync_tlb(stack_page, page_size);
    if (st != STATUS_OK) {
      return st;
    }
  }
  user_entry = entry;
  user_sp = stack_base + k_user_stack_bytes - 16ULL;
#endif

  st = mm_translate(user_entry, &pa, &flags);
  if (st == STATUS_OK) {
    kprintf("usermode: entry=0x%llx pa=0x%llx flags=0x%llx\n", user_entry, (u64)pa, flags);
  }

  st = uaccess_set_user_window(0ULL, ~0ULL);
  if (st != STATUS_OK) {
    return st;
  }
  st = arch_user_mode_set_kernel_stack(&g_usermode_kernel_stack[sizeof(g_usermode_kernel_stack)]);
  if (st != STATUS_OK) {
    return st;
  }

  kprintf("usermode: launching init task\n");
  arch_user_mode_enter((arch_user_entry_t)(uptr)user_entry, (void *)0, user_sp);
  return STATUS_FAULT;
}
#endif

static const char *boot_info_arch_name(u64 arch_id) {
  switch (arch_id) {
  case BOOT_INFO_ARCH_X86_64:
    return "x86_64";
  case BOOT_INFO_ARCH_ARM64:
    return "arm64";
  case BOOT_INFO_ARCH_RISCV64:
    return "riscv64";
  default:
    return "unknown";
  }
}

#if MONOLITH_TIMER_SELFTEST
static u64 timer_selftest_spins_for_arch(u64 arch_id) {
  if (arch_id == BOOT_INFO_ARCH_X86_64) {
    return MONOLITH_TIMER_SELFTEST_SPINS;
  }
  return 20000000ULL;
}

static int timer_progress_selftest(u64 spins) {
  u64 start = timer_ticks();
  u64 last = start;
  u64 i;

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
  status_t cpu_status;
  status_t page_status;
  status_t heap_status;
  status_t percpu_status;
  status_t discovery_status;
  status_t bus_status;
  status_t device_status;
  status_t driver_reg_status;
  status_t smp_status;
  status_t pci_status;
  status_t usb_status;
  status_t domain_status;
  status_t net_status;
  status_t audio_status;
  status_t resource_status;
  status_t irq_status;
  status_t irq_domain_status;
  status_t timer_status;
  status_t sched_status;
  status_t trace_status;
  status_t input_status;
  status_t arch_input_status;
  status_t dma_status;
  status_t iommu_status;
  status_t syscall_status;
  status_t syscall_probe_status;
  status_t uaccess_negative_status;
  status_t user_bootstrap_status;
  status_t console_status;
  time_quality_t tq;
  const hw_desc_t *hw;
  syscall_response_t syscall_probe_resp = {STATUS_DEFERRED, 0};

  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ") We good!\n");
  capability_profile_print();
  capability_domain_dump_matrix();
  if (boot_info == (const boot_info_t *)0) {
    arch_puts("boot_info: null\n");
    goto spin;
  }

  panic_set_context(boot_info);
  trace_status = trace_init(boot_info);
  input_status = input_init();
  arch_input_status = arch_input_init(boot_info);
  if (status_is_ok(input_status) && status_is_ok(arch_input_status)) {
    kprintf("input: backend ready (arch=%s drop=%llu)\n", boot_info_arch_name(boot_info->arch_id), input_drop_count());
  }
  if (status_is_ok(trace_status)) {
    trace_emit(TRACE_CLASS_DEVICE, 0xB007ULL, boot_info->arch_id, boot_info->abi_version);
  }
  mutable_boot_info = (boot_info_t *)boot_info;
  cpu_status = arch_cpu_early_init(boot_info);
  percpu_status = percpu_init_boot_cpu(boot_info);
  discovery_status = hw_discovery_init(boot_info);
  hw = hw_desc_get();
  bus_status = device_bus_init(boot_info, hw);
  resource_status = hw_resource_init(boot_info);
  pci_status = pci_enumerate(boot_info);
  usb_status = usb_enumerate(boot_info);
  domain_status = device_domains_enumerate(boot_info);
  net_status = net_enumerate(boot_info);
  audio_status = audio_enumerate(boot_info);
  smp_status = smp_init(boot_info);
  mem_status = arch_mm_early_init(mutable_boot_info);
  dma_status = dma_init(boot_info);
  iommu_status = iommu_init(boot_info);
  page_status = page_alloc_init(mutable_boot_info);
  heap_status = kmalloc_init(mutable_boot_info);
  driver_registry_reset();
  driver_set_boot_info(boot_info);
  driver_reg_status = device_model_register_builtin_drivers();
  device_status = driver_probe_all(hw);
  trace_emit(TRACE_CLASS_DEVICE, (u64)device_status, device_bus_count(), boot_info->arch_id);
  irq_status = driver_class_last_status("irqc");
  irq_domain_status = irq_domain_init(boot_info);
  timer_status = driver_class_last_status("timer");
  sched_status = sched_init();
  syscall_status = syscall_init(boot_info);
  /*
   * Arm64 firmware/runtime combinations can be sensitive during early SVC probe.
   * Keep bootstrap deterministic by probing via kernel dispatch on arm64.
   */
  if (boot_info->arch_id == BOOT_INFO_ARCH_ARM64) {
    syscall_probe_status = syscall_invoke_kernel(SYSCALL_OP_ABI_INFO, 0, 0, 0, 0, 0, 0, &syscall_probe_resp);
  } else {
    syscall_probe_status = syscall_invoke_trap(SYSCALL_OP_ABI_INFO, 0, 0, 0, 0, 0, 0, &syscall_probe_resp);
  }
  (void)uaccess_set_user_window(0x2000ULL, 0x1000ULL);
  uaccess_negative_status = syscall_invoke_kernel(SYSCALL_OP_DEBUG_LOG, 0x1000ULL, 5ULL, 0, 0, 0, 0, &syscall_probe_resp);
  (void)uaccess_set_user_window(0ULL, 0ULL);
  {
    user_task_bootstrap_t user_ctx;
    user_bootstrap_status = user_task_bootstrap_prepare(boot_info, &user_ctx);
    kprintf("usermode: launching init task st=%s (%d)\n", status_str(user_bootstrap_status), user_bootstrap_status);
  }
  console_status = driver_class_last_status("console");
  kprintf("time: now_ns=%llu ticks=%llu hz=%llu\n", time_now_ns(), time_ticks(), time_hz());
  if (time_quality(&tq) == STATUS_OK) {
    kprintf("time: quality stable=%llu unstable=%llu emulated=%llu cal_hz=%llu drift_ppm<=%llu cross_cpu=%llu/%llu\n",
            tq.stable, tq.unstable, tq.emulated, tq.calibrated_hz, tq.drift_ppm_bound, tq.cross_cpu_passed,
            tq.cross_cpu_checked);
  }

  if (!status_is_ok(cpu_status) && cpu_status != STATUS_DEFERRED) {
    kprintf("arch_cpu_early_init: %s (%d)\n", status_str(cpu_status), cpu_status);
  }
  if (!status_is_ok(mem_status) && mem_status != STATUS_DEFERRED) {
    kprintf("arch_mm_early_init: %s (%d)\n", status_str(mem_status), mem_status);
  }
  if (!status_is_ok(dma_status) && dma_status != STATUS_DEFERRED) {
    kprintf("dma_init: %s (%d)\n", status_str(dma_status), dma_status);
  }
  if (!status_is_ok(iommu_status) && iommu_status != STATUS_DEFERRED) {
    kprintf("iommu_init: %s (%d)\n", status_str(iommu_status), iommu_status);
  }
  if (!status_is_ok(percpu_status) && percpu_status != STATUS_DEFERRED) {
    kprintf("percpu_init_boot_cpu: %s (%d)\n", status_str(percpu_status), percpu_status);
  }
  if (!status_is_ok(discovery_status) && discovery_status != STATUS_DEFERRED) {
    kprintf("hw_discovery_init: %s (%d)\n", status_str(discovery_status), discovery_status);
  }
  if (!status_is_ok(bus_status) && bus_status != STATUS_DEFERRED) {
    kprintf("device_bus_init: %s (%d)\n", status_str(bus_status), bus_status);
  }
  if (!status_is_ok(resource_status) && resource_status != STATUS_DEFERRED) {
    kprintf("hw_resource_init: %s (%d)\n", status_str(resource_status), resource_status);
  }
  if (!status_is_ok(pci_status) && pci_status != STATUS_DEFERRED) {
    kprintf("pci_enumerate: %s (%d)\n", status_str(pci_status), pci_status);
  }
  if (!status_is_ok(usb_status) && usb_status != STATUS_DEFERRED) {
    kprintf("usb_enumerate: %s (%d)\n", status_str(usb_status), usb_status);
  }
  if (!status_is_ok(domain_status) && domain_status != STATUS_DEFERRED) {
    kprintf("device_domains_enumerate: %s (%d)\n", status_str(domain_status), domain_status);
  }
  if (!status_is_ok(net_status) && net_status != STATUS_DEFERRED) {
    kprintf("net_enumerate: %s (%d)\n", status_str(net_status), net_status);
  }
  if (!status_is_ok(audio_status) && audio_status != STATUS_DEFERRED) {
    kprintf("audio_enumerate: %s (%d)\n", status_str(audio_status), audio_status);
  }
  if (!status_is_ok(smp_status) && smp_status != STATUS_DEFERRED) {
    kprintf("smp_init: %s (%d)\n", status_str(smp_status), smp_status);
  }
  if (!status_is_ok(driver_reg_status) && driver_reg_status != STATUS_DEFERRED) {
    kprintf("device_model_register_builtin_drivers: %s (%d)\n", status_str(driver_reg_status), driver_reg_status);
  }
  if (!status_is_ok(device_status) && device_status != STATUS_DEFERRED) {
    kprintf("driver_probe_all: %s (%d)\n", status_str(device_status), device_status);
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
  if (!status_is_ok(irq_domain_status) && irq_domain_status != STATUS_DEFERRED) {
    kprintf("irq_domain_init: %s (%d)\n", status_str(irq_domain_status), irq_domain_status);
  }
  if (!status_is_ok(timer_status) && timer_status != STATUS_DEFERRED) {
    kprintf("timer_init: %s (%d)\n", status_str(timer_status), timer_status);
  }
  if (!status_is_ok(sched_status) && sched_status != STATUS_DEFERRED) {
    kprintf("sched_init: %s (%d)\n", status_str(sched_status), sched_status);
  } else if (status_is_ok(sched_status)) {
    task_t *cur = sched_current();
    kprintf("sched: init ok current_tid=%llu\n", cur != (task_t *)0 ? cur->tid : 0ULL);
    sched_tick();
  }
  if (!status_is_ok(syscall_status) && syscall_status != STATUS_DEFERRED) {
    kprintf("syscall_init: %s (%d)\n", status_str(syscall_status), syscall_status);
  }
  if (!status_is_ok(trace_status) && trace_status != STATUS_DEFERRED) {
    kprintf("trace_init: %s (%d)\n", status_str(trace_status), trace_status);
  }
  if (!status_is_ok(input_status) && input_status != STATUS_DEFERRED) {
    kprintf("input_init: %s (%d)\n", status_str(input_status), input_status);
  }
  if (!status_is_ok(arch_input_status) && arch_input_status != STATUS_DEFERRED) {
    kprintf("arch_input_init: %s (%d)\n", status_str(arch_input_status), arch_input_status);
  }
  kprintf("personality: active=%s id=0x%llx\n", personality_active_name(), personality_active_id());
  if (!status_is_ok(syscall_probe_status) && syscall_probe_status != STATUS_DEFERRED) {
    kprintf("syscall abi probe call: %s (%d)\n", status_str(syscall_probe_status), syscall_probe_status);
  } else if (status_is_ok(syscall_probe_status)) {
    kprintf("syscall abi probe: call=%s (%d) resp=%s (%d) value=0x%llx\n", status_str(syscall_probe_status),
            syscall_probe_status, status_str(syscall_probe_resp.status), syscall_probe_resp.status,
            syscall_probe_resp.value);
  } else {
    kprintf("syscall abi probe: call=%s (%d)\n", status_str(syscall_probe_status), syscall_probe_status);
  }
  kprintf("uaccess negative probe: %s (%d)\n", status_str(uaccess_negative_status), uaccess_negative_status);
  if (!status_is_ok(console_status) && console_status != STATUS_DEFERRED) {
    kprintf("console_init: %s (%d)\n", status_str(console_status), console_status);
  }

#if MONOLITH_TIMER_SELFTEST
  if (timer_status == STATUS_OK) {
    u64 spins = timer_selftest_spins_for_arch(boot_info->arch_id);
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
    u32 i;

    kmalloc_stats(&st);
    kprintf("kmalloc dbg: begin used=%llu free=%llu allocs=%llu frees=%llu\n", st.bytes_used, st.bytes_free,
            st.alloc_count, st.free_count);

    for (i = 0; i < 8U; ++i) {
      ptrs[i] = kmalloc(64 + (u64)i * 16ULL);
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
#if MONOLITH_USERMODE_PROBE
#if defined(__x86_64__) || defined(__aarch64__) || defined(__riscv)
  if (status_is_ok(mem_status) && status_is_ok(page_status) && status_is_ok(syscall_status)) {
    status_t launch_st = usermode_probe_launch(boot_info);
    if (!status_is_ok(launch_st) && launch_st != STATUS_DEFERRED) {
      kprintf("usermode launch: %s (%d)\n", status_str(launch_st), launch_st);
    }
  }
#endif
#endif
  kprintf("smp: possible=%llu online=%llu\n", smp_cpu_count_possible(), smp_cpu_count_online());
  {
    status_t ipi_st = ipi_send(0ULL, IPI_KIND_RESCHEDULE);
    status_t tlb_st = tlb_shootdown(1ULL, 0ULL, 4096ULL);
    kprintf("smp: ipi_self=%s(%d) tlb_local=%s(%d)\n", status_str(ipi_st), ipi_st, status_str(tlb_st), tlb_st);
  }
  {
    if (hw != (const hw_desc_t *)0) {
      kprintf("hw: source=0x%llx cpus=%llu irqc=%llu timers=%llu mmio=%llu uarts=%llu\n", hw->source_mask,
              hw->cpu_count, hw->irq_controller_count, hw->timer_count, hw->mmio_region_count, hw->uart_count);
    }
  }
  device_bus_dump();
  device_report_dump_all();
  device_report_dump_class(DEVICE_CLASS_BLOCK);
  device_report_dump_class(DEVICE_CLASS_INPUT);
  device_report_dump_class(DEVICE_CLASS_DISPLAY);
  device_report_dump_class(DEVICE_CLASS_NET);
  device_report_dump_class(DEVICE_CLASS_AUDIO);
  kprintf("pci: devices=%llu\n", pci_device_count());
  kprintf("usb: hosts=%llu devices=%llu\n", usb_host_count(), usb_device_count());
  kprintf("domains: block=%llu input=%llu display=%llu\n", block_device_count(), input_device_count(),
          display_device_count());
  kprintf("net: devices=%llu\n", net_device_count());
  kprintf("audio: devices=%llu\n", audio_device_count());
  net_dump_diagnostics();
  audio_dump_diagnostics();
  syscall_dump_table();

#if MONOLITH_BOOTINFO_DEBUG
  diag_boot_info_print(boot_info);
#endif

  kprintf("time: now_ns=%llu ticks=%llu hz=%llu\n", time_now_ns(), time_ticks(), time_hz());
spin:
  for (;;) {
    arch_cpu_halt();
  }
}
