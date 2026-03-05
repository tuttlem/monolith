#ifndef KERNEL_UACCESS_H
#define KERNEL_UACCESS_H

#include "kernel.h"

status_t uaccess_set_user_window(u64 base, u64 size);
status_t copy_from_user_checked(void *dst, u64 user_src, u64 len);
status_t copy_to_user_checked(u64 user_dst, const void *src, u64 len);

#endif
