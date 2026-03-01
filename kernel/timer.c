#include "timer.h"
#include "arch_timer.h"
#include "interrupts.h"

typedef struct {
  BOOT_U64 initialized;
  BOOT_U64 hz;
  BOOT_U64 irq_vector;
  volatile BOOT_U64 ticks;
} timer_state_t;

static timer_state_t g_timer;

static void timer_irq_handler(const interrupt_frame_t *frame, void *ctx) {
  (void)ctx;
  if (frame == (const interrupt_frame_t *)0) {
    return;
  }

  g_timer.ticks += 1ULL;
  arch_timer_ack(frame->vector);
}

status_t timer_init(const boot_info_t *boot_info) {
  BOOT_U64 hz = 0;
  BOOT_U64 vector = 0;
  status_t st;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  g_timer.initialized = 0;
  g_timer.hz = 0;
  g_timer.irq_vector = 0;
  g_timer.ticks = 0;

  st = arch_timer_init(boot_info, &hz, &vector);
  if (st == STATUS_DEFERRED) {
    return STATUS_DEFERRED;
  }
  if (st != STATUS_OK) {
    return st;
  }

  if (hz == 0ULL || vector >= INTERRUPT_MAX_VECTORS) {
    return STATUS_FAULT;
  }

  st = interrupts_register_handler_owned(vector, timer_irq_handler, (void *)0, "timer");
  if (st != STATUS_OK) {
    return st;
  }

  g_timer.hz = hz;
  g_timer.irq_vector = vector;
  g_timer.initialized = 1ULL;
  interrupts_enable();
  return STATUS_OK;
}

BOOT_U64 timer_ticks(void) { return g_timer.ticks; }

BOOT_U64 timer_hz(void) { return g_timer.hz; }
