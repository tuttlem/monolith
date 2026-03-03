#ifndef KERNEL_DMA_H
#define KERNEL_DMA_H

#include "boot_info.h"
#include "status.h"

typedef BOOT_U64 dma_addr_t;

typedef enum {
  DMA_DIR_TO_DEVICE = 0,
  DMA_DIR_FROM_DEVICE = 1,
  DMA_DIR_BIDIRECTIONAL = 2
} dma_dir_t;

typedef struct {
  BOOT_U64 max_addr;
  BOOT_U64 max_segment_len;
  BOOT_U64 alignment;
} dma_constraints_t;

status_t dma_init(const boot_info_t *boot_info);
status_t dma_map(BOOT_U64 device_id, void *cpu_ptr, BOOT_U64 len, dma_dir_t dir, dma_addr_t *out);
status_t dma_unmap(BOOT_U64 device_id, dma_addr_t addr, BOOT_U64 len, dma_dir_t dir);
status_t dma_sync_for_device(BOOT_U64 device_id, dma_addr_t addr, BOOT_U64 len, dma_dir_t dir);
status_t dma_sync_for_cpu(BOOT_U64 device_id, dma_addr_t addr, BOOT_U64 len, dma_dir_t dir);
status_t dma_set_constraints(BOOT_U64 device_id, const dma_constraints_t *constraints);
status_t dma_get_constraints(BOOT_U64 device_id, dma_constraints_t *out_constraints);

#endif
