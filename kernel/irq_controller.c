#include "irq_controller.h"
#include "irq_domain.h"

#define IRQ_DESC_FLAG_MSI 0x1ULL

static struct {
  BOOT_U64 registered;
  const char *name;
  irq_controller_ops_t ops;
} g_irqc;

static struct {
  BOOT_U64 initialized;
  BOOT_U64 alloc_count;
  irq_desc_t allocs[IRQ_DOMAIN_MAX_ALLOCS];
} g_irq_domain;

void irq_controller_reset(void) {
  g_irqc.registered = 0;
  g_irqc.name = (const char *)0;
  g_irqc.ops.enable_irq = (status_t(*)(BOOT_U64))0;
  g_irqc.ops.disable_irq = (status_t(*)(BOOT_U64))0;
  g_irqc.ops.ack_irq = (void (*)(BOOT_U64))0;
  g_irqc.ops.eoi_irq = (void (*)(BOOT_U64))0;
  g_irqc.ops.map_irq = (status_t(*)(BOOT_U64, BOOT_U64 *))0;
  g_irqc.ops.vector_to_irq = (status_t(*)(BOOT_U64, BOOT_U64 *))0;
  g_irq_domain.initialized = 0;
  g_irq_domain.alloc_count = 0;
}

status_t irq_controller_register(const char *name, const irq_controller_ops_t *ops) {
  if (name == (const char *)0 || ops == (const irq_controller_ops_t *)0 || ops->map_irq == (status_t(*)(BOOT_U64, BOOT_U64 *))0 ||
      ops->vector_to_irq == (status_t(*)(BOOT_U64, BOOT_U64 *))0) {
    return STATUS_INVALID_ARG;
  }
  if (g_irqc.registered != 0) {
    return STATUS_BUSY;
  }

  g_irqc.name = name;
  g_irqc.ops.enable_irq = ops->enable_irq;
  g_irqc.ops.disable_irq = ops->disable_irq;
  g_irqc.ops.ack_irq = ops->ack_irq;
  g_irqc.ops.eoi_irq = ops->eoi_irq;
  g_irqc.ops.map_irq = ops->map_irq;
  g_irqc.ops.vector_to_irq = ops->vector_to_irq;
  g_irqc.registered = 1;
  return STATUS_OK;
}

const char *irq_controller_name(void) { return g_irqc.name; }

status_t irq_controller_enable(BOOT_U64 irq) {
  if (g_irqc.registered == 0 || g_irqc.ops.enable_irq == (status_t(*)(BOOT_U64))0) {
    return STATUS_DEFERRED;
  }
  return g_irqc.ops.enable_irq(irq);
}

status_t irq_controller_disable(BOOT_U64 irq) {
  if (g_irqc.registered == 0 || g_irqc.ops.disable_irq == (status_t(*)(BOOT_U64))0) {
    return STATUS_DEFERRED;
  }
  return g_irqc.ops.disable_irq(irq);
}

void irq_controller_ack(BOOT_U64 irq) {
  if (g_irqc.registered == 0 || g_irqc.ops.ack_irq == (void (*)(BOOT_U64))0) {
    return;
  }
  g_irqc.ops.ack_irq(irq);
}

void irq_controller_eoi(BOOT_U64 irq) {
  if (g_irqc.registered == 0 || g_irqc.ops.eoi_irq == (void (*)(BOOT_U64))0) {
    return;
  }
  g_irqc.ops.eoi_irq(irq);
}

status_t irq_controller_map(BOOT_U64 irq, BOOT_U64 *out_vector) {
  if (out_vector == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_irqc.registered == 0 || g_irqc.ops.map_irq == (status_t(*)(BOOT_U64, BOOT_U64 *))0) {
    return STATUS_DEFERRED;
  }
  return g_irqc.ops.map_irq(irq, out_vector);
}

status_t irq_controller_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq) {
  if (out_irq == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_irqc.registered == 0 || g_irqc.ops.vector_to_irq == (status_t(*)(BOOT_U64, BOOT_U64 *))0) {
    return STATUS_DEFERRED;
  }
  return g_irqc.ops.vector_to_irq(vector, out_irq);
}

status_t irq_domain_init(const boot_info_t *boot_info) {
  BOOT_U64 i;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  g_irq_domain.initialized = 1;
  g_irq_domain.alloc_count = 0;
  for (i = 0; i < IRQ_DOMAIN_MAX_ALLOCS; ++i) {
    g_irq_domain.allocs[i].global_irq = 0;
    g_irq_domain.allocs[i].hwirq = 0;
    g_irq_domain.allocs[i].vector = 0;
    g_irq_domain.allocs[i].flags = 0;
    g_irq_domain.allocs[i].owner_device_id = 0;
  }
  return STATUS_OK;
}

status_t irq_alloc_line(BOOT_U64 device_id, BOOT_U64 hwirq, irq_desc_t *out) {
  irq_desc_t *slot;
  BOOT_U64 vector;
  status_t st;

  if (!g_irq_domain.initialized) {
    return STATUS_DEFERRED;
  }
  if (out == (irq_desc_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_irq_domain.alloc_count >= IRQ_DOMAIN_MAX_ALLOCS) {
    return STATUS_NO_MEMORY;
  }

  st = irq_controller_map(hwirq, &vector);
  if (st != STATUS_OK) {
    return st;
  }
  slot = &g_irq_domain.allocs[g_irq_domain.alloc_count];
  slot->global_irq = g_irq_domain.alloc_count;
  slot->hwirq = hwirq;
  slot->vector = vector;
  slot->flags = 0;
  slot->owner_device_id = device_id;
  out->global_irq = slot->global_irq;
  out->hwirq = slot->hwirq;
  out->vector = slot->vector;
  out->flags = slot->flags;
  out->owner_device_id = slot->owner_device_id;
  g_irq_domain.alloc_count += 1ULL;
  return STATUS_OK;
}

status_t irq_alloc_msi(BOOT_U64 device_id, BOOT_U64 nvec, irq_desc_t *out_vec) {
  BOOT_U64 i;

  if (!g_irq_domain.initialized) {
    return STATUS_DEFERRED;
  }
  if (out_vec == (irq_desc_t *)0 || nvec == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  if (g_irq_domain.alloc_count + nvec > IRQ_DOMAIN_MAX_ALLOCS) {
    return STATUS_NO_MEMORY;
  }

  for (i = 0ULL; i < nvec; ++i) {
    irq_desc_t *slot = &g_irq_domain.allocs[g_irq_domain.alloc_count];
    slot->global_irq = g_irq_domain.alloc_count;
    slot->hwirq = 0ULL;
    slot->vector = 0ULL;
    slot->flags = IRQ_DESC_FLAG_MSI;
    slot->owner_device_id = device_id;
    out_vec[i].global_irq = slot->global_irq;
    out_vec[i].hwirq = slot->hwirq;
    out_vec[i].vector = slot->vector;
    out_vec[i].flags = slot->flags;
    out_vec[i].owner_device_id = slot->owner_device_id;
    g_irq_domain.alloc_count += 1ULL;
  }
  return STATUS_OK;
}

status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg) {
  (void)irq;
  (void)cfg;
  return STATUS_OK;
}

status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask) {
  (void)irq;
  (void)mask;
  return STATUS_OK;
}

BOOT_U64 irq_domain_alloc_count(void) { return g_irq_domain.alloc_count; }
