#include "irq_controller.h"

static struct {
  BOOT_U64 registered;
  const char *name;
  irq_controller_ops_t ops;
} g_irqc;

void irq_controller_reset(void) {
  g_irqc.registered = 0;
  g_irqc.name = (const char *)0;
  g_irqc.ops.enable_irq = (status_t(*)(BOOT_U64))0;
  g_irqc.ops.disable_irq = (status_t(*)(BOOT_U64))0;
  g_irqc.ops.ack_irq = (void (*)(BOOT_U64))0;
  g_irqc.ops.eoi_irq = (void (*)(BOOT_U64))0;
  g_irqc.ops.map_irq = (status_t(*)(BOOT_U64, BOOT_U64 *))0;
  g_irqc.ops.vector_to_irq = (status_t(*)(BOOT_U64, BOOT_U64 *))0;
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
