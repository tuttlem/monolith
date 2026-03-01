#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include "boot_info.h"

void kmain(const boot_info_t *boot_info);
int kprintf(const char *fmt, ...);
void arch_puts(const char *s);
void arch_halt(void);

#endif
