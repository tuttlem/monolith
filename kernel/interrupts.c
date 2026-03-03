#include "interrupts.h"
#include "arch_irq.h"
#include "config.h"
#include "hw_desc.h"
#include "irq_controller.h"
#include "panic.h"
#include "percpu.h"
#include "trace.h"

static const char g_irq_owner_anon[] = "anonymous";

static BOOT_U64 exception_class_from_frame(const interrupt_frame_t *frame) {
  if (frame == (const interrupt_frame_t *)0) {
    return EXCEPTION_CLASS_UNKNOWN;
  }
  if (frame->vector >= 32ULL) {
    return EXCEPTION_CLASS_IRQ;
  }
  return EXCEPTION_CLASS_FAULT;
}

static const char *x86_64_exception_name(BOOT_U64 vector) {
  switch (vector) {
  case 0:
    return "divide_error";
  case 1:
    return "debug";
  case 2:
    return "nmi";
  case 3:
    return "breakpoint";
  case 4:
    return "overflow";
  case 5:
    return "bound_range_exceeded";
  case 6:
    return "invalid_opcode";
  case 7:
    return "device_not_available";
  case 8:
    return "double_fault";
  case 10:
    return "invalid_tss";
  case 11:
    return "segment_not_present";
  case 12:
    return "stack_segment_fault";
  case 13:
    return "general_protection";
  case 14:
    return "page_fault";
  case 16:
    return "x87_fpu_error";
  case 17:
    return "alignment_check";
  case 18:
    return "machine_check";
  case 19:
    return "simd_floating_point";
  case 20:
    return "virtualization";
  default:
    return "x86_exception";
  }
}

static const char *arm64_exception_name(const interrupt_frame_t *frame) {
  BOOT_U64 ec;
  if (frame == (const interrupt_frame_t *)0) {
    return "arm64_exception";
  }

  if (frame->vector >= 32ULL) {
    if (frame->vector == 59ULL) {
      return "timer_interrupt";
    }
    return "irq_interrupt";
  }

  ec = (frame->error_code >> 26) & 0x3FULL;
  switch (ec) {
  case 0x15ULL:
    return "svc_aarch64";
  case 0x20ULL:
  case 0x21ULL:
    return "instruction_abort";
  case 0x22ULL:
  case 0x24ULL:
  case 0x25ULL:
    return "data_abort";
  case 0x2FULL:
    return "serror";
  case 0x3CULL:
    return "brk";
  default:
    return "arm64_exception";
  }
}

static const char *riscv64_exception_name(const interrupt_frame_t *frame) {
  BOOT_U64 scause;
  BOOT_U64 is_interrupt;
  BOOT_U64 code;

  if (frame == (const interrupt_frame_t *)0) {
    return "riscv_exception";
  }

  scause = frame->error_code;
  is_interrupt = (scause >> 63) & 1ULL;
  code = scause & ((1ULL << 63) - 1ULL);
  if (is_interrupt != 0) {
    switch (code) {
    case 5ULL:
      return "timer_interrupt";
    case 9ULL:
      return "external_interrupt";
    default:
      return "interrupt";
    }
  }

  switch (code) {
  case 2ULL:
    return "illegal_instruction";
  case 3ULL:
    return "breakpoint";
  case 12ULL:
    return "instruction_page_fault";
  case 13ULL:
    return "load_page_fault";
  case 15ULL:
    return "store_page_fault";
  default:
    return "riscv_exception";
  }
}

static const char *exception_reason_from_frame(const interrupt_frame_t *frame) {
  if (frame == (const interrupt_frame_t *)0) {
    return "exception:null_frame";
  }

  switch (frame->arch_id) {
  case BOOT_INFO_ARCH_X86_64:
    return x86_64_exception_name(frame->vector);
  case BOOT_INFO_ARCH_ARM64:
    return arm64_exception_name(frame);
  case BOOT_INFO_ARCH_RISCV64:
    return riscv64_exception_name(frame);
  default:
    return "unhandled_exception";
  }
}

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
  info.class_id = exception_class_from_frame(frame);
  info.vector = frame->vector;
  info.error_code = frame->error_code;
  info.raw_syndrome = frame->error_code;
  info.fault_addr = frame->fault_addr;
  info.ip = frame->ip;
  info.sp = frame->sp;
  info.flags = frame->flags;
  info.reason = why;
  panic_from_exception(&info);
}

static void default_interrupt_handler(const interrupt_frame_t *frame) {
  BOOT_U64 irq = 0;

  if (frame == (const interrupt_frame_t *)0) {
    return;
  }

  if (frame->vector < 32ULL) {
    interrupts_panic_exception(frame, exception_reason_from_frame(frame));
    return;
  }

  /*
   * Early-timer bootstrap window:
   * - x86_64 can observe legacy PIT vector 32 before timer handler ownership.
   * - arm64 can observe virtual timer vector 59 before timer handler ownership.
   */
  if ((frame->arch_id == BOOT_INFO_ARCH_X86_64 && frame->vector == 32ULL) ||
      (frame->arch_id == BOOT_INFO_ARCH_ARM64 && frame->vector == 59ULL)) {
    if (irq_controller_vector_to_irq(frame->vector, &irq) == STATUS_OK) {
      irq_controller_eoi(irq);
    }
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
  const hw_desc_t *hw;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  g_interrupts.initialized = 0;
  g_interrupts.arch_id = boot_info->arch_id;
  hw = hw_desc_get();
  if (hw != (const hw_desc_t *)0 && hw->irq_controller_count == 0ULL) {
    return STATUS_DEFERRED;
  }
  irq_controller_reset();
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

  if (irq_controller_name() != (const char *)0) {
    kprintf("irq-controller: %s\n", irq_controller_name());
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
  percpu_t *cpu;

  if (g_interrupts.initialized == 0 || frame == (const interrupt_frame_t *)0) {
    return;
  }

  trace_emit(TRACE_CLASS_IRQ_ENTRY, frame->vector, frame->error_code, frame->arch_id);

  cpu = percpu_current();
  if (cpu != (percpu_t *)0) {
    cpu->irq_nesting += 1ULL;
  }
  if (frame->vector >= INTERRUPT_MAX_VECTORS) {
    interrupts_panic_exception(frame, "invalid_vector");
    goto out;
  }

  slot = g_interrupts.slots[frame->vector];
  if (slot.fn == (interrupt_handler_t)0) {
    default_interrupt_handler(frame);
  } else {
    slot.fn(frame, slot.ctx);
  }

out:
  trace_emit(TRACE_CLASS_IRQ_EXIT, frame->vector, frame->error_code, frame->arch_id);
  if (cpu != (percpu_t *)0 && cpu->irq_nesting != 0ULL) {
    cpu->irq_nesting -= 1ULL;
  }
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
