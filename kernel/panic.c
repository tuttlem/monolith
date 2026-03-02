#include "panic.h"
#include "arch_cpu.h"
#include "arch_irq.h"
#include "print.h"

static volatile BOOT_U64 g_panic_active = 0;
static BOOT_U64 g_panic_arch_id = 0;
static BOOT_U64 g_panic_boot_cpu_id = 0;

static void panic_stop_forever(void) {
  for (;;) {
    arch_panic_stop();
  }
}

void panic(const char *reason) {
  if (g_panic_active != 0) {
    panic_stop_forever();
  }

  g_panic_active = 1;
  arch_irq_disable();
  kprintf("panic: %s arch=%llu cpu=%llu\n", reason == (const char *)0 ? "unknown" : reason, g_panic_arch_id,
          arch_cpu_id() == 0ULL ? g_panic_boot_cpu_id : arch_cpu_id());
  kprintf("panic: stacktrace: <stub>\n");
  panic_stop_forever();
}

void panicf(const char *fmt, ...) {
  char msg[256];
  va_list args;

  if (g_panic_active != 0) {
    panic_stop_forever();
  }

  g_panic_active = 1;
  arch_irq_disable();

  va_start(args, fmt);
  kvsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  kprintf("panic: %s arch=%llu cpu=%llu\n", msg, g_panic_arch_id,
          arch_cpu_id() == 0ULL ? g_panic_boot_cpu_id : arch_cpu_id());
  kprintf("panic: stacktrace: <stub>\n");
  panic_stop_forever();
}

void panic_from_exception(const exception_info_t *info) {
  if (g_panic_active != 0) {
    panic_stop_forever();
  }
  g_panic_active = 1;
  arch_irq_disable();

  if (info == (const exception_info_t *)0) {
    kprintf("panic: exception: <null> arch=%llu cpu=%llu\n", g_panic_arch_id,
            arch_cpu_id() == 0ULL ? g_panic_boot_cpu_id : arch_cpu_id());
    kprintf("panic: stacktrace: <stub>\n");
    panic_stop_forever();
  }

  kprintf(
      "panic: exception: %s arch=%llu cpu=%llu vector=%llu err=0x%llx ip=0x%llx sp=0x%llx flags=0x%llx fault=0x%llx\n",
      info->reason == (const char *)0 ? "unknown" : info->reason, info->arch_id, arch_cpu_id(), info->vector,
      info->error_code, info->ip, info->sp, info->flags, info->fault_addr);
  kprintf("panic: stacktrace: <stub>\n");
  panic_stop_forever();
}

void panic_set_context(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0) {
    return;
  }
  g_panic_arch_id = boot_info->arch_id;
  g_panic_boot_cpu_id = boot_info->boot_cpu_id;
}
