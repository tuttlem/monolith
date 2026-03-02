#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include "kernel.h"

typedef struct {
  BOOT_U64 class_id;
  BOOT_U64 arch_id;
  BOOT_U64 vector;
  BOOT_U64 error_code;
  BOOT_U64 raw_syndrome;
  BOOT_U64 fault_addr;
  BOOT_U64 ip;
  BOOT_U64 sp;
  BOOT_U64 flags;
  const char *reason;
} exception_info_t;

#define EXCEPTION_CLASS_UNKNOWN 0ULL
#define EXCEPTION_CLASS_FAULT 1ULL
#define EXCEPTION_CLASS_TRAP 2ULL
#define EXCEPTION_CLASS_ABORT 3ULL
#define EXCEPTION_CLASS_IRQ 4ULL

void panic(const char *reason);
void panicf(const char *fmt, ...);
void panic_from_exception(const exception_info_t *info);
void panic_set_context(const boot_info_t *boot_info);

#endif
