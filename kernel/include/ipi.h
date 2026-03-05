#ifndef KERNEL_IPI_H
#define KERNEL_IPI_H

#include "kernel.h"

typedef u64 cpu_mask_t;
typedef u64 virt_addr_t;

typedef enum {
  IPI_KIND_RESCHEDULE = 1,
  IPI_KIND_TLB_SHOOTDOWN = 2,
  IPI_KIND_CALL_FUNC = 3
} ipi_kind_t;

status_t ipi_send(u64 cpu_id, ipi_kind_t kind);
status_t tlb_shootdown(cpu_mask_t mask, virt_addr_t va, u64 len);

#endif
