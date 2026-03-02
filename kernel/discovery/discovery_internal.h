#ifndef KERNEL_DISCOVERY_INTERNAL_H
#define KERNEL_DISCOVERY_INTERNAL_H

#include "hw_desc.h"

void hw_discovery_parse_acpi(const boot_info_t *boot_info, hw_desc_t *desc);
void hw_discovery_parse_dtb(const boot_info_t *boot_info, hw_desc_t *desc);

#endif
