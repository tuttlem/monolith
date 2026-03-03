#ifndef KERNEL_DEVICE_BUS_H
#define KERNEL_DEVICE_BUS_H

#include "hw_desc.h"

#define DEVICE_BUS_API_VERSION_MAJOR 1U
#define DEVICE_BUS_API_VERSION_MINOR 0U

#define DEVICE_BUS_MAX_BUSES 16U
#define DEVICE_BUS_MAX_DEVICES 256U
#define DEVICE_BUS_MAX_RESOURCES 8U

#define DEVICE_BUS_ID_NONE (~0ULL)

typedef enum {
  BUS_TYPE_ROOT = 0,
  BUS_TYPE_PLATFORM = 1,
  BUS_TYPE_PCI = 2,
  BUS_TYPE_USB = 3
} bus_type_t;

typedef enum {
  DEVICE_CLASS_UNKNOWN = 0,
  DEVICE_CLASS_IRQC,
  DEVICE_CLASS_TIMER,
  DEVICE_CLASS_CONSOLE,
  DEVICE_CLASS_MMIO,
  DEVICE_CLASS_FRAMEBUFFER,
  DEVICE_CLASS_PCI_DEVICE,
  DEVICE_CLASS_USB_HOST,
  DEVICE_CLASS_USB_DEVICE,
  DEVICE_CLASS_BLOCK,
  DEVICE_CLASS_NET,
  DEVICE_CLASS_INPUT,
  DEVICE_CLASS_DISPLAY,
  DEVICE_CLASS_AUDIO
} device_class_t;

typedef enum {
  DEVICE_RESOURCE_NONE = 0,
  DEVICE_RESOURCE_MMIO,
  DEVICE_RESOURCE_IRQ,
  DEVICE_RESOURCE_IOPORT,
  DEVICE_RESOURCE_DMA
} device_resource_kind_t;

typedef struct {
  device_resource_kind_t kind;
  BOOT_U64 base;
  BOOT_U64 size;
  BOOT_U64 flags;
} device_resource_t;

typedef struct {
  BOOT_U64 id;
  BOOT_U64 parent_id;
  BOOT_U64 bus_id;
  const char *name;
  device_class_t class_id;
  BOOT_U64 vendor_id;
  BOOT_U64 device_id;
  BOOT_U64 revision;
  BOOT_U64 class_code;
  BOOT_U64 subclass_code;
  BOOT_U64 prog_if;
  BOOT_U64 resource_count;
  device_resource_t resources[DEVICE_BUS_MAX_RESOURCES];
  void *driver_data;
} device_t;

typedef struct {
  BOOT_U64 id;
  BOOT_U64 parent_bus_id;
  bus_type_t type;
  const char *name;
} bus_t;

status_t device_bus_init(const boot_info_t *boot_info, const hw_desc_t *hw);
void device_bus_reset(void);

status_t device_bus_register_bus(const bus_t *bus_template, BOOT_U64 *out_bus_id);
status_t device_bus_register_device(const device_t *dev_template, BOOT_U64 *out_device_id);

const bus_t *device_bus_get_bus(BOOT_U64 bus_id);
const device_t *device_bus_get_device(BOOT_U64 device_id);
BOOT_U64 device_bus_count(void);
const device_t *device_bus_device_at(BOOT_U64 index);

BOOT_U64 device_bus_find_first_by_class(device_class_t class_id);
BOOT_U64 device_bus_find_next_by_class(device_class_t class_id, BOOT_U64 after_id);

void device_bus_dump(void);

/* Spec 131 hardware-resource manager compatibility layer. */
status_t device_bus_replace_resources(BOOT_U64 device_id, const device_resource_t *resources, BOOT_U64 count);

#endif
