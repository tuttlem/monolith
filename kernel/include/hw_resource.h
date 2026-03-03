#ifndef KERNEL_HW_RESOURCE_H
#define KERNEL_HW_RESOURCE_H

#include "device_bus.h"

typedef enum {
  HW_RESOURCE_NONE = 0,
  HW_RESOURCE_MMIO,
  HW_RESOURCE_IRQ,
  HW_RESOURCE_IOPORT,
  HW_RESOURCE_DMA,
  HW_RESOURCE_CLOCK,
  HW_RESOURCE_RESET
} hw_resource_type_t;

typedef struct {
  hw_resource_type_t type;
  BOOT_U64 base;
  BOOT_U64 size;
  BOOT_U64 flags;
} hw_resource_t;

typedef struct {
  BOOT_U64 device_id;
  BOOT_U64 total_count;
} device_resource_view_t;

status_t hw_resource_init(const boot_info_t *boot_info);
status_t hw_resource_attach(BOOT_U64 device_id, const hw_resource_t *list, BOOT_U64 count);
status_t hw_resource_get(BOOT_U64 device_id, hw_resource_type_t type, BOOT_U64 index, hw_resource_t *out);
BOOT_U64 hw_resource_count(BOOT_U64 device_id, hw_resource_type_t type);
status_t hw_resource_view(BOOT_U64 device_id, device_resource_view_t *out);

#endif
