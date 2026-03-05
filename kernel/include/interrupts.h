#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include "kernel.h"

#define INTERRUPT_MAX_VECTORS 256ULL

typedef struct {
  u64 arch_id;
  u64 vector;
  u64 error_code;
  u64 fault_addr;
  u64 ip;
  u64 sp;
  u64 flags;
} interrupt_frame_t;

typedef void (*interrupt_handler_t)(const interrupt_frame_t *frame, void *ctx);

status_t interrupts_init(const boot_info_t *boot_info);
status_t interrupts_register_handler(u64 vector, interrupt_handler_t handler, void *ctx);
status_t interrupts_register_handler_owned(u64 vector, interrupt_handler_t handler, void *ctx, const char *owner);
status_t interrupts_unregister_handler(u64 vector, const char *owner);
const char *interrupts_handler_owner(u64 vector);
void interrupts_dispatch(const interrupt_frame_t *frame);
void interrupts_enable(void);
void interrupts_disable(void);

#endif
