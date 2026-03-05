#ifndef KERNEL_IRQ_CONTROLLER_H
#define KERNEL_IRQ_CONTROLLER_H

#include "kernel.h"

typedef struct {
  status_t (*enable_irq)(u64 irq);
  status_t (*disable_irq)(u64 irq);
  void (*ack_irq)(u64 irq);
  void (*eoi_irq)(u64 irq);
  status_t (*map_irq)(u64 irq, u64 *out_vector);
  status_t (*vector_to_irq)(u64 vector, u64 *out_irq);
} irq_controller_ops_t;

void irq_controller_reset(void);
status_t irq_controller_register(const char *name, const irq_controller_ops_t *ops);
const char *irq_controller_name(void);
status_t irq_controller_enable(u64 irq);
status_t irq_controller_disable(u64 irq);
void irq_controller_ack(u64 irq);
void irq_controller_eoi(u64 irq);
status_t irq_controller_map(u64 irq, u64 *out_vector);
status_t irq_controller_vector_to_irq(u64 vector, u64 *out_irq);

#endif
