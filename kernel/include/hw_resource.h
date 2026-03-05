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
  u64 base;
  u64 size;
  u64 flags;
} hw_resource_t;

typedef struct {
  u64 device_id;
  u64 total_count;
} device_resource_view_t;

status_t hw_resource_init(const boot_info_t *boot_info);
status_t hw_resource_attach(u64 device_id, const hw_resource_t *list, u64 count);
status_t hw_resource_get(u64 device_id, hw_resource_type_t type, u64 index, hw_resource_t *out);
u64 hw_resource_count(u64 device_id, hw_resource_type_t type);
status_t hw_resource_view(u64 device_id, device_resource_view_t *out);

#endif
