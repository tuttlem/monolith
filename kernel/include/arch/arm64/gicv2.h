#ifndef KERNEL_ARCH_ARM64_GICV2_H
#define KERNEL_ARCH_ARM64_GICV2_H

#include "kernel.h"

status_t arm64_gicv2_controller_init(const boot_info_t *boot_info);
status_t arm64_gicv2_claim_irq(u64 *out_irq);
void arm64_gicv2_eoi_irq(u64 irq);

#endif
