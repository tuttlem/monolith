#ifndef KERNEL_UACCESS_H
#define KERNEL_UACCESS_H

#include "kernel.h"

status_t uaccess_set_user_window(BOOT_U64 base, BOOT_U64 size);
status_t copy_from_user_checked(void *dst, BOOT_U64 user_src, BOOT_U64 len);
status_t copy_to_user_checked(BOOT_U64 user_dst, const void *src, BOOT_U64 len);

#endif
