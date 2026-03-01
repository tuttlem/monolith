#ifndef KERNEL_ARCH_X86_64_EARLY_PAGING_H
#define KERNEL_ARCH_X86_64_EARLY_PAGING_H

#include "boot_info.h"

typedef struct {
  BOOT_U64 old_cr3;
  BOOT_U64 new_cr3;
  BOOT_U64 identity_bytes_mapped;
} x86_64_early_paging_result_t;

int x86_64_early_paging_takeover(x86_64_early_paging_result_t *result);

#endif
