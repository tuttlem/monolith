#include "interrupts.h"
#include "arch_interrupts.h"

typedef struct {
  interrupt_handler_t fn;
  void *ctx;
} interrupt_slot_t;

static struct {
  BOOT_U64 initialized;
  BOOT_U64 arch_id;
  interrupt_slot_t slots[INTERRUPT_MAX_VECTORS];
} g_interrupts;

static void default_interrupt_handler(const interrupt_frame_t *frame) {
  if (frame == (const interrupt_frame_t *)0) {
    return;
  }

  kprintf("interrupt: vector=%llu err=0x%llx ip=0x%llx sp=0x%llx flags=0x%llx fault=0x%llx\n", frame->vector,
          frame->error_code, frame->ip, frame->sp, frame->flags, frame->fault_addr);

  if (frame->vector < 32ULL) {
    kprintf("exception: halting on unhandled exception vector %llu\n", frame->vector);
    for (;;) {
      arch_halt();
    }
  }
}

status_t interrupts_init(const boot_info_t *boot_info) {
  BOOT_U64 i;
  status_t st;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  g_interrupts.initialized = 0;
  g_interrupts.arch_id = boot_info->arch_id;
  for (i = 0; i < INTERRUPT_MAX_VECTORS; ++i) {
    g_interrupts.slots[i].fn = (interrupt_handler_t)0;
    g_interrupts.slots[i].ctx = (void *)0;
  }

  st = arch_interrupts_init(boot_info);
  if (st != STATUS_OK && st != STATUS_DEFERRED) {
    return st;
  }

  g_interrupts.initialized = 1;
  return st;
}

status_t interrupts_register_handler(BOOT_U64 vector, interrupt_handler_t handler, void *ctx) {
  if (g_interrupts.initialized == 0) {
    return STATUS_DEFERRED;
  }
  if (vector >= INTERRUPT_MAX_VECTORS || handler == (interrupt_handler_t)0) {
    return STATUS_INVALID_ARG;
  }

  g_interrupts.slots[vector].fn = handler;
  g_interrupts.slots[vector].ctx = ctx;
  return STATUS_OK;
}

void interrupts_dispatch(const interrupt_frame_t *frame) {
  interrupt_slot_t slot;

  if (g_interrupts.initialized == 0 || frame == (const interrupt_frame_t *)0) {
    return;
  }
  if (frame->vector >= INTERRUPT_MAX_VECTORS) {
    default_interrupt_handler(frame);
    return;
  }

  slot = g_interrupts.slots[frame->vector];
  if (slot.fn == (interrupt_handler_t)0) {
    default_interrupt_handler(frame);
    return;
  }

  slot.fn(frame, slot.ctx);
}

void interrupts_enable(void) {
  if (g_interrupts.initialized == 0) {
    return;
  }
  arch_interrupts_enable();
}

void interrupts_disable(void) {
  if (g_interrupts.initialized == 0) {
    return;
  }
  arch_interrupts_disable();
}
