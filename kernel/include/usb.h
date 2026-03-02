#ifndef KERNEL_USB_H
#define KERNEL_USB_H

#include "device_bus.h"

status_t usb_enumerate(const boot_info_t *boot_info);
BOOT_U64 usb_host_count(void);
BOOT_U64 usb_device_count(void);

#endif
