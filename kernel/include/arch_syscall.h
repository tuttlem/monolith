#ifndef KERNEL_ARCH_SYSCALL_H
#define KERNEL_ARCH_SYSCALL_H

#include "kernel.h"

/*
 * Architecture syscall trap entry hook.
 * STATUS_OK: arch trap entry path is active.
 * STATUS_DEFERRED: trap entry path is intentionally not wired yet.
 */
status_t arch_syscall_init(const boot_info_t *boot_info);
status_t arch_syscall_get_vector(BOOT_U64 *out_vector);
status_t arch_syscall_trigger(void);

#endif
