#ifndef KERNEL_ARCH_SPARC64_EARLY_PAGING_H
#define KERNEL_ARCH_SPARC64_EARLY_PAGING_H

#include "boot_info.h"

typedef struct {
  BOOT_U64 old_root;
  BOOT_U64 new_root;
  BOOT_U64 identity_bytes_mapped;
} sparc64_early_paging_result_t;

int sparc64_early_paging_takeover(sparc64_early_paging_result_t *result);

#endif
