#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include "kernel.h"

#define INTERRUPT_MAX_VECTORS 256ULL

typedef struct {
  BOOT_U64 arch_id;
  BOOT_U64 vector;
  BOOT_U64 error_code;
  BOOT_U64 fault_addr;
  BOOT_U64 ip;
  BOOT_U64 sp;
  BOOT_U64 flags;
} interrupt_frame_t;

typedef void (*interrupt_handler_t)(const interrupt_frame_t *frame, void *ctx);

status_t interrupts_init(const boot_info_t *boot_info);
status_t interrupts_register_handler(BOOT_U64 vector, interrupt_handler_t handler, void *ctx);
void interrupts_dispatch(const interrupt_frame_t *frame);
void interrupts_enable(void);
void interrupts_disable(void);

#endif
