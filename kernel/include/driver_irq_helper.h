#ifndef KERNEL_DRIVER_IRQ_HELPER_H
#define KERNEL_DRIVER_IRQ_HELPER_H

#include "interrupts.h"
#include "irq_controller.h"

static inline void driver_irq_complete(const interrupt_frame_t *frame) {
  u64 irq;
  if (frame == (const interrupt_frame_t *)0) {
    return;
  }
  if (irq_controller_vector_to_irq(frame->vector, &irq) == STATUS_OK) {
    irq_controller_eoi(irq);
  }
}

#endif
