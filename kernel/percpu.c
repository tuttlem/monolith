#include "percpu.h"
#include "arch_cpu.h"

static percpu_t g_percpu[PERCPU_MAX_CPUS];
static BOOT_U64 g_percpu_online_count = 0;
static BOOT_U64 g_percpu_initialized = 0;

status_t percpu_init_boot_cpu(const boot_info_t *boot_info) {
  BOOT_U64 cpu_id;
  percpu_t *cpu0;
  status_t st;
  BOOT_U32 i;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  for (i = 0; i < PERCPU_MAX_CPUS; ++i) {
    g_percpu[i].cpu_id = 0;
    g_percpu[i].online = 0;
    g_percpu[i].irq_nesting = 0;
    g_percpu[i].preempt_disable_depth = 0;
    g_percpu[i].local_tick_count = 0;
    g_percpu[i].current_task = (void *)0;
    g_percpu[i].arch_local = (void *)0;
  }

  cpu_id = arch_cpu_id();
  if ((boot_info->valid_mask & BOOT_INFO_HAS_BOOT_CPU_ID) != 0) {
    cpu_id = boot_info->boot_cpu_id;
  }

  cpu0 = &g_percpu[0];
  cpu0->cpu_id = cpu_id;
  cpu0->online = 1ULL;

  st = arch_cpu_set_local_base((BOOT_U64)(BOOT_UPTR)cpu0);
  if (st != STATUS_OK) {
    return st;
  }

  g_percpu_online_count = 1ULL;
  g_percpu_initialized = 1ULL;
  return STATUS_OK;
}

status_t percpu_register_current_cpu(BOOT_U64 cpu_id) {
  BOOT_U64 i;
  BOOT_U64 slot_index = 0;
  BOOT_U64 free_found = 0;
  percpu_t *slot;
  status_t st;

  if (g_percpu_initialized == 0) {
    return STATUS_DEFERRED;
  }

  for (i = 0; i < g_percpu_online_count; ++i) {
    if (g_percpu[i].online != 0 && g_percpu[i].cpu_id == cpu_id) {
      st = arch_cpu_set_local_base((BOOT_U64)(BOOT_UPTR)&g_percpu[i]);
      if (st != STATUS_OK) {
        return st;
      }
      return STATUS_OK;
    }
  }

  for (i = 0; i < PERCPU_MAX_CPUS; ++i) {
    if (g_percpu[i].online == 0) {
      slot_index = i;
      free_found = 1ULL;
      break;
    }
  }
  if (free_found == 0ULL) {
    return STATUS_BUSY;
  }

  slot = &g_percpu[slot_index];
  slot->cpu_id = cpu_id;
  slot->online = 1ULL;
  slot->irq_nesting = 0;
  slot->preempt_disable_depth = 0;
  slot->local_tick_count = 0;
  slot->current_task = (void *)0;
  slot->arch_local = (void *)0;

  st = arch_cpu_set_local_base((BOOT_U64)(BOOT_UPTR)slot);
  if (st != STATUS_OK) {
    slot->online = 0;
    return st;
  }

  if (slot_index + 1ULL > g_percpu_online_count) {
    g_percpu_online_count = slot_index + 1ULL;
  }
  return STATUS_OK;
}

percpu_t *percpu_current(void) {
  BOOT_U64 base;

  if (g_percpu_initialized == 0) {
    return (percpu_t *)0;
  }

  base = arch_cpu_get_local_base();
  if (base == 0ULL) {
    return (percpu_t *)0;
  }
  return (percpu_t *)(BOOT_UPTR)base;
}

percpu_t *percpu_by_id(BOOT_U64 cpu_id) {
  BOOT_U64 i;

  if (g_percpu_initialized == 0) {
    return (percpu_t *)0;
  }

  for (i = 0; i < g_percpu_online_count; ++i) {
    if (g_percpu[i].online != 0 && g_percpu[i].cpu_id == cpu_id) {
      return &g_percpu[i];
    }
  }
  return (percpu_t *)0;
}

BOOT_U64 percpu_online_count(void) {
  BOOT_U64 i;
  BOOT_U64 count = 0;

  if (g_percpu_initialized == 0) {
    return 0ULL;
  }

  for (i = 0; i < g_percpu_online_count; ++i) {
    if (g_percpu[i].online != 0) {
      count += 1ULL;
    }
  }
  return count;
}
