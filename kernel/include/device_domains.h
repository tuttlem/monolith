#ifndef KERNEL_DEVICE_DOMAINS_H
#define KERNEL_DEVICE_DOMAINS_H

#include "device_bus.h"

status_t device_domains_enumerate(const boot_info_t *boot_info);
BOOT_U64 block_device_count(void);
BOOT_U64 input_device_count(void);
BOOT_U64 display_device_count(void);

#endif
