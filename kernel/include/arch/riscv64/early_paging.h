#ifndef KERNEL_ARCH_RISCV64_EARLY_PAGING_H
#define KERNEL_ARCH_RISCV64_EARLY_PAGING_H

#include "boot_info.h"

typedef struct {
  u64 old_satp;
  u64 new_satp;
  u64 identity_bytes_mapped;
} riscv64_early_paging_result_t;

int riscv64_early_paging_takeover(riscv64_early_paging_result_t *result);

#endif
