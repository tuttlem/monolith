#ifndef KERNEL_ARCH_INPUT_H
#define KERNEL_ARCH_INPUT_H

#include "kernel.h"

status_t arch_input_init(const boot_info_t *boot_info);
void arch_input_poll(void);

#endif
