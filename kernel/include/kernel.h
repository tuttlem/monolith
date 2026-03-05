#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include "types.h"
#include "boot_info.h"
#include "status.h"

void kmain(const boot_info_t *boot_info);
int kprintf(const char *fmt, ...);
void arch_puts(const char *s);
void arch_halt(void);
void arch_panic_stop(void);
void arch_exception_selftest_trigger(void);

#endif
