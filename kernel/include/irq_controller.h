#ifndef KERNEL_IRQ_CONTROLLER_H
#define KERNEL_IRQ_CONTROLLER_H

#include "kernel.h"

typedef struct {
  status_t (*enable_irq)(BOOT_U64 irq);
  status_t (*disable_irq)(BOOT_U64 irq);
  void (*ack_irq)(BOOT_U64 irq);
  void (*eoi_irq)(BOOT_U64 irq);
  status_t (*map_irq)(BOOT_U64 irq, BOOT_U64 *out_vector);
  status_t (*vector_to_irq)(BOOT_U64 vector, BOOT_U64 *out_irq);
} irq_controller_ops_t;

void irq_controller_reset(void);
status_t irq_controller_register(const char *name, const irq_controller_ops_t *ops);
const char *irq_controller_name(void);
status_t irq_controller_enable(BOOT_U64 irq);
status_t irq_controller_disable(BOOT_U64 irq);
void irq_controller_ack(BOOT_U64 irq);
void irq_controller_eoi(BOOT_U64 irq);
status_t irq_controller_map(BOOT_U64 irq, BOOT_U64 *out_vector);
status_t irq_controller_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq);

#endif
