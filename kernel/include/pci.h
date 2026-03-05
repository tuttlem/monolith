#ifndef KERNEL_PCI_H
#define KERNEL_PCI_H

#include "device_bus.h"

status_t pci_enumerate(const boot_info_t *boot_info);
u64 pci_device_count(void);

#endif
