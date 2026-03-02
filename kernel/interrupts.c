#include "interrupts.h"
#include "arch_irq.h"
#include "panic.h"

#ifndef MONOLITH_IRQ_LOG_UNHANDLED
#define MONOLITH_IRQ_LOG_UNHANDLED 1
#endif

static const char g_irq_owner_anon[] = "anonymous";

typedef struct {
  interrupt_handler_t fn;
  void *ctx;
  const char *owner;
} interrupt_slot_t;

static struct {
  BOOT_U64 initialized;
  BOOT_U64 arch_id;
  interrupt_slot_t slots[INTERRUPT_MAX_VECTORS];
  BOOT_U64 unhandled_once[INTERRUPT_MAX_VECTORS];
} g_interrupts;

static int owner_eq(const char *a, const char *b) {
  if (a == b) {
    return 1;
  }
  if (a == (const char *)0 || b == (const char *)0) {
    return 0;
  }
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == *b;
}

static void interrupts_panic_exception(const interrupt_frame_t *frame, const char *why) {
  exception_info_t info;

  if (frame == (const interrupt_frame_t *)0) {
    panic("exception:null_frame");
    return;
  }

  info.arch_id = frame->arch_id;
  info.vector = frame->vector;
  info.error_code = frame->error_code;
  info.fault_addr = frame->fault_addr;
  info.ip = frame->ip;
  info.sp = frame->sp;
  info.flags = frame->flags;
  info.reason = why;
  panic_from_exception(&info);
}

static void default_interrupt_handler(const interrupt_frame_t *frame) {
  if (frame == (const interrupt_frame_t *)0) {
    return;
  }

  if (frame->vector < 32ULL) {
    interrupts_panic_exception(frame, "unhandled");
    return;
  }

#if MONOLITH_IRQ_LOG_UNHANDLED
  if (frame->vector < INTERRUPT_MAX_VECTORS && g_interrupts.unhandled_once[frame->vector] == 0ULL) {
    g_interrupts.unhandled_once[frame->vector] = 1ULL;
    kprintf("interrupt: unhandled vector=%llu (suppressing repeats)\n", frame->vector);
  }
#endif
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
    g_interrupts.slots[i].owner = (const char *)0;
    g_interrupts.unhandled_once[i] = 0ULL;
  }

  st = arch_irq_init(boot_info);
  if (st != STATUS_OK && st != STATUS_DEFERRED) {
    return st;
  }

  g_interrupts.initialized = 1;
  return st;
}

status_t interrupts_register_handler_owned(BOOT_U64 vector, interrupt_handler_t handler, void *ctx, const char *owner) {
  interrupt_slot_t *slot;
  const char *new_owner;

  if (g_interrupts.initialized == 0) {
    return STATUS_DEFERRED;
  }
  if (vector >= INTERRUPT_MAX_VECTORS || handler == (interrupt_handler_t)0) {
    return STATUS_INVALID_ARG;
  }

  new_owner = (owner == (const char *)0) ? g_irq_owner_anon : owner;
  slot = &g_interrupts.slots[vector];
  if (slot->fn != (interrupt_handler_t)0 && !owner_eq(slot->owner, new_owner)) {
    return STATUS_BUSY;
  }

  slot->fn = handler;
  slot->ctx = ctx;
  slot->owner = new_owner;
  g_interrupts.unhandled_once[vector] = 0ULL;
  return STATUS_OK;
}

status_t interrupts_register_handler(BOOT_U64 vector, interrupt_handler_t handler, void *ctx) {
  return interrupts_register_handler_owned(vector, handler, ctx, g_irq_owner_anon);
}

status_t interrupts_unregister_handler(BOOT_U64 vector, const char *owner) {
  interrupt_slot_t *slot;
  const char *expected_owner;

  if (g_interrupts.initialized == 0) {
    return STATUS_DEFERRED;
  }
  if (vector >= INTERRUPT_MAX_VECTORS) {
    return STATUS_INVALID_ARG;
  }

  slot = &g_interrupts.slots[vector];
  if (slot->fn == (interrupt_handler_t)0) {
    return STATUS_NOT_FOUND;
  }

  expected_owner = (owner == (const char *)0) ? g_irq_owner_anon : owner;
  if (!owner_eq(slot->owner, expected_owner)) {
    return STATUS_BUSY;
  }

  slot->fn = (interrupt_handler_t)0;
  slot->ctx = (void *)0;
  slot->owner = (const char *)0;
  return STATUS_OK;
}

const char *interrupts_handler_owner(BOOT_U64 vector) {
  if (g_interrupts.initialized == 0 || vector >= INTERRUPT_MAX_VECTORS) {
    return (const char *)0;
  }
  return g_interrupts.slots[vector].owner;
}

void interrupts_dispatch(const interrupt_frame_t *frame) {
  interrupt_slot_t slot;

  if (g_interrupts.initialized == 0 || frame == (const interrupt_frame_t *)0) {
    return;
  }
  if (frame->vector >= INTERRUPT_MAX_VECTORS) {
    interrupts_panic_exception(frame, "invalid_vector");
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
  arch_irq_enable();
}

void interrupts_disable(void) {
  if (g_interrupts.initialized == 0) {
    return;
  }
  arch_irq_disable();
}
