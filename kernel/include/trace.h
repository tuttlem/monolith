#ifndef KERNEL_TRACE_H
#define KERNEL_TRACE_H

#include "kernel.h"

typedef enum {
  TRACE_CLASS_IRQ_ENTRY = 1,
  TRACE_CLASS_IRQ_EXIT = 2,
  TRACE_CLASS_SYSCALL = 3,
  TRACE_CLASS_TIMER = 4,
  TRACE_CLASS_PANIC = 5,
  TRACE_CLASS_DEVICE = 6
} trace_class_t;

typedef struct trace_record {
  BOOT_U64 seq;
  BOOT_U64 timestamp_ns;
  BOOT_U64 cpu_id;
  trace_class_t cls;
  BOOT_U64 a0;
  BOOT_U64 a1;
  BOOT_U64 a2;
} trace_record_t;

typedef void (*trace_sink_t)(const trace_record_t *record);

status_t trace_init(const boot_info_t *boot_info);
void trace_emit(trace_class_t cls, BOOT_U64 a0, BOOT_U64 a1, BOOT_U64 a2);
status_t trace_dump(trace_sink_t sink);
void trace_sink_kprintf(const trace_record_t *record);

#endif
