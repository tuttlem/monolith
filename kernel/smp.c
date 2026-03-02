#include "smp.h"
#include "arch_cpu.h"
#include "arch_smp.h"
#include "config.h"
#include "percpu.h"
#include "print.h"

#define SMP_WAIT_LOOPS 10000000ULL

static struct {
  BOOT_U64 initialized;
  BOOT_U64 possible_cpus;
  BOOT_U64 online_cpus;
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

  possible = arch_cpu_count_hint();
  if (possible == 0ULL) {
    possible = 1ULL;
  }

  g_smp.possible_cpus = possible;
  g_smp.online_cpus = percpu_online_count();

#if !MONOLITH_FEATURE_SMP
  g_smp.initialized = 1ULL;
  return STATUS_DEFERRED;
#endif

  st = arch_smp_bootstrap(boot_info, &possible, &started);
  if (st == STATUS_DEFERRED) {
    g_smp.initialized = 1ULL;
    if (possible != 0ULL) {
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
