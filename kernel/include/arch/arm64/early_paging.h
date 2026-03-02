#ifndef KERNEL_ARCH_ARM64_EARLY_PAGING_H
#define KERNEL_ARCH_ARM64_EARLY_PAGING_H

#include "boot_info.h"

typedef struct {
  BOOT_U64 old_ttbr0;
  BOOT_U64 new_ttbr0;
  BOOT_U64 identity_bytes_mapped;
} arm64_early_paging_result_t;

int arm64_early_paging_takeover(const boot_info_t *boot_info, arm64_early_paging_result_t *result);

#endif
