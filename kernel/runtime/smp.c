#include "smp.h"
#include "arch_cpu.h"
#include "arch_smp.h"
#include "cpu_caps.h"
#include "cpu_context.h"
#include "config.h"
#include "hw_desc.h"
#include "percpu.h"
#include "print.h"

#define SMP_WAIT_LOOPS 10000000ULL

static struct {
  BOOT_U64 initialized;
  BOOT_U64 possible_cpus;
  BOOT_U64 online_cpus;
  BOOT_U64 cpu_online_bitmap;
} g_smp;

static BOOT_U64 smp_wait_for_online(BOOT_U64 expected) {
  BOOT_U64 i;
  BOOT_U64 online = percpu_online_count();

  if (expected <= online) {
    return online;
  }

  for (i = 0; i < SMP_WAIT_LOOPS; ++i) {
    arch_cpu_relax();
    online = percpu_online_count();
    if (online >= expected) {
      break;
    }
  }
  return online;
}

status_t smp_init(const boot_info_t *boot_info) {
  status_t st;
  BOOT_U64 possible = 1;
  BOOT_U64 started = 0;
  BOOT_U64 online;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_smp.initialized != 0) {
    return STATUS_OK;
  }

  possible = hw_desc_cpu_count_hint();
  if (possible == 0ULL) {
    possible = arch_cpu_count_hint();
  }
  if (possible == 0ULL) {
    possible = 1ULL;
  }

  g_smp.possible_cpus = possible;
  g_smp.online_cpus = percpu_online_count();
  g_smp.cpu_online_bitmap = 1ULL;

#if !MONOLITH_FEATURE_SMP
  g_smp.initialized = 1ULL;
  return STATUS_DEFERRED;
#endif

  st = arch_smp_bootstrap(boot_info, &possible, &started);
  if (st == STATUS_DEFERRED) {
    g_smp.initialized = 1ULL;
    if (possible > g_smp.possible_cpus) {
      g_smp.possible_cpus = possible;
    }
    g_smp.online_cpus = percpu_online_count();
    return STATUS_DEFERRED;
  }
  if (st != STATUS_OK) {
    return st;
  }

  if (possible == 0ULL) {
    possible = g_smp.possible_cpus;
  }
  if (possible < 1ULL) {
    possible = 1ULL;
  }
  g_smp.possible_cpus = possible;

  online = smp_wait_for_online(1ULL + started);
  g_smp.online_cpus = online;
  if (online >= 64ULL) {
    g_smp.cpu_online_bitmap = ~0ULL;
  } else {
    g_smp.cpu_online_bitmap = (1ULL << online) - 1ULL;
  }
  g_smp.initialized = 1ULL;

  if (online < 1ULL + started) {
    kprintf("smp: timeout waiting for secondaries (started=%llu online=%llu)\n", started, online);
    return STATUS_TRY_AGAIN;
  }

  kprintf("smp: possible=%llu online=%llu started=%llu\n", g_smp.possible_cpus, g_smp.online_cpus, started);
  return STATUS_OK;
}

BOOT_U64 smp_cpu_count_online(void) {
  BOOT_U64 online = percpu_online_count();
  if (online > 0ULL) {
    return online;
  }
  return g_smp.online_cpus;
}

BOOT_U64 smp_cpu_count_possible(void) {
  if (g_smp.possible_cpus != 0ULL) {
    return g_smp.possible_cpus;
  }
  return 1ULL;
}

status_t smp_cpu_start(BOOT_U64 cpu_id) {
  status_t st;

  if (g_smp.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (cpu_id >= g_smp.possible_cpus) {
    return STATUS_NOT_FOUND;
  }
  if (cpu_id == 0ULL || cpu_id < percpu_online_count()) {
    return STATUS_OK;
  }
  st = arch_smp_cpu_start(cpu_id);
  if (st == STATUS_OK && cpu_id < 64ULL) {
    g_smp.cpu_online_bitmap |= (1ULL << cpu_id);
    g_smp.online_cpus = percpu_online_count();
  }
  return st;
}

status_t ipi_send(BOOT_U64 cpu_id, ipi_kind_t kind) {
  if (g_smp.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (cpu_id >= g_smp.possible_cpus) {
    return STATUS_NOT_FOUND;
  }
  return arch_smp_ipi_send(cpu_id, (BOOT_U64)kind);
}

status_t tlb_shootdown(cpu_mask_t mask, virt_addr_t va, BOOT_U64 len) {
  if (g_smp.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (len == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  return arch_smp_tlb_shootdown((BOOT_U64)mask, (BOOT_U64)va, len);
}

void smp_secondary_entry(BOOT_U64 cpu_id) {
  if (percpu_register_current_cpu(cpu_id) != STATUS_OK) {
    for (;;) {
      arch_cpu_halt();
    }
  }

  for (;;) {
    arch_cpu_halt();
  }
}

status_t cpu_caps_query(cpu_caps_t *out_caps) {
  if (out_caps == (cpu_caps_t *)0) {
    return STATUS_INVALID_ARG;
  }

#if defined(__x86_64__)
  out_caps->arch_id = BOOT_INFO_ARCH_X86_64;
  out_caps->has_fp = 1ULL;
  out_caps->has_simd = 1ULL;
  out_caps->has_virtualization = 1ULL;
  out_caps->has_atomic = 1ULL;
#elif defined(__aarch64__)
  out_caps->arch_id = BOOT_INFO_ARCH_ARM64;
  out_caps->has_fp = 1ULL;
  out_caps->has_simd = 1ULL;
  out_caps->has_virtualization = 1ULL;
  out_caps->has_atomic = 1ULL;
#elif defined(__riscv)
  out_caps->arch_id = BOOT_INFO_ARCH_RISCV64;
  out_caps->has_fp = 0ULL;
  out_caps->has_simd = 0ULL;
  out_caps->has_virtualization = 0ULL;
  out_caps->has_atomic = 1ULL;
#else
  out_caps->arch_id = BOOT_INFO_ARCH_UNKNOWN;
  out_caps->has_fp = 0ULL;
  out_caps->has_simd = 0ULL;
  out_caps->has_virtualization = 0ULL;
  out_caps->has_atomic = 0ULL;
#endif

  out_caps->has_cycle_counter = (arch_cycle_counter() != 0ULL) ? 1ULL : 0ULL;
  return STATUS_OK;
}

status_t cpu_context_init(cpu_context_t *ctx, void (*entry)(void *), void *arg, void *stack_top) {
  if (ctx == (cpu_context_t *)0 || entry == (void (*)(void *))0 || stack_top == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  ctx->sp = (BOOT_U64)(BOOT_UPTR)stack_top;
  ctx->ip = (BOOT_U64)(BOOT_UPTR)entry;
  ctx->arg = (BOOT_U64)(BOOT_UPTR)arg;
  ctx->flags = 0ULL;
  return STATUS_OK;
}

status_t cpu_context_switch(cpu_context_t *from, cpu_context_t *to) {
  cpu_context_t tmp;

  if (from == (cpu_context_t *)0 || to == (cpu_context_t *)0) {
    return STATUS_INVALID_ARG;
  }

  tmp.sp = from->sp;
  tmp.ip = from->ip;
  tmp.arg = from->arg;
  tmp.flags = from->flags;

  from->sp = to->sp;
  from->ip = to->ip;
  from->arg = to->arg;
  from->flags = to->flags;

  to->sp = tmp.sp;
  to->ip = tmp.ip;
  to->arg = tmp.arg;
  to->flags = tmp.flags;
  return STATUS_OK;
}
