#include "device_bus.h"
#include "hw_resource.h"
#include "print.h"

static bus_t g_buses[DEVICE_BUS_MAX_BUSES];
static device_t g_devices[DEVICE_BUS_MAX_DEVICES];
static u64 g_bus_count;
static u64 g_device_count;
static u64 g_initialized;

static u64 g_root_bus_id = DEVICE_BUS_ID_NONE;
static u64 g_platform_bus_id = DEVICE_BUS_ID_NONE;
static device_hotplug_fn_t g_hotplug_on_add;
static device_hotplug_fn_t g_hotplug_on_remove;
static void *g_hotplug_ctx;

static void init_empty_device(device_t *dev) {
  u64 j;
  dev->id = DEVICE_BUS_ID_NONE;
  dev->parent_id = DEVICE_BUS_ID_NONE;
  dev->bus_id = DEVICE_BUS_ID_NONE;
  dev->name = (const char *)0;
  dev->class_id = DEVICE_CLASS_UNKNOWN;
  dev->vendor_id = 0;
  dev->device_id = 0;
  dev->revision = 0;
  dev->class_code = 0;
  dev->subclass_code = 0;
  dev->prog_if = 0;
  dev->resource_count = 0;
  dev->driver_data = (void *)0;
  dev->active = 0;
  for (j = 0; j < DEVICE_BUS_MAX_RESOURCES; ++j) {
    dev->resources[j].kind = DEVICE_RESOURCE_NONE;
    dev->resources[j].base = 0;
    dev->resources[j].size = 0;
    dev->resources[j].flags = 0;
  }
}

static const char *class_name(device_class_t class_id) {
  switch (class_id) {
  case DEVICE_CLASS_IRQC:
    return "irqc";
  case DEVICE_CLASS_TIMER:
    return "timer";
  case DEVICE_CLASS_CONSOLE:
    return "console";
  case DEVICE_CLASS_MMIO:
    return "mmio";
  case DEVICE_CLASS_FRAMEBUFFER:
    return "framebuffer";
  case DEVICE_CLASS_PCI_DEVICE:
    return "pci-dev";
  case DEVICE_CLASS_USB_HOST:
    return "usb-host";
  case DEVICE_CLASS_USB_DEVICE:
    return "usb-dev";
  case DEVICE_CLASS_BLOCK:
    return "block";
  case DEVICE_CLASS_NET:
    return "net";
  case DEVICE_CLASS_INPUT:
    return "input";
  case DEVICE_CLASS_DISPLAY:
    return "display";
  case DEVICE_CLASS_AUDIO:
    return "audio";
  default:
    return "unknown";
  }
}

static const char *bus_name(bus_type_t type) {
  switch (type) {
  case BUS_TYPE_ROOT:
    return "root";
  case BUS_TYPE_PLATFORM:
    return "platform";
  case BUS_TYPE_PCI:
    return "pci";
  case BUS_TYPE_USB:
    return "usb";
  case BUS_TYPE_VIRTIO:
    return "virtio";
  default:
    return "unknown";
  }
}

void device_bus_reset(void) {
  u64 i;
  u64 j;

  for (i = 0; i < DEVICE_BUS_MAX_BUSES; ++i) {
    g_buses[i].id = DEVICE_BUS_ID_NONE;
    g_buses[i].parent_bus_id = DEVICE_BUS_ID_NONE;
    g_buses[i].type = BUS_TYPE_ROOT;
    g_buses[i].name = (const char *)0;
  }

  for (i = 0; i < DEVICE_BUS_MAX_DEVICES; ++i) {
    g_devices[i].id = DEVICE_BUS_ID_NONE;
    g_devices[i].parent_id = DEVICE_BUS_ID_NONE;
    g_devices[i].bus_id = DEVICE_BUS_ID_NONE;
    g_devices[i].name = (const char *)0;
    g_devices[i].class_id = DEVICE_CLASS_UNKNOWN;
    g_devices[i].vendor_id = 0;
    g_devices[i].device_id = 0;
    g_devices[i].revision = 0;
    g_devices[i].class_code = 0;
    g_devices[i].subclass_code = 0;
    g_devices[i].prog_if = 0;
    g_devices[i].resource_count = 0;
    g_devices[i].driver_data = (void *)0;
    g_devices[i].active = 0;
    for (j = 0; j < DEVICE_BUS_MAX_RESOURCES; ++j) {
      g_devices[i].resources[j].kind = DEVICE_RESOURCE_NONE;
      g_devices[i].resources[j].base = 0;
      g_devices[i].resources[j].size = 0;
      g_devices[i].resources[j].flags = 0;
    }
  }

  g_bus_count = 0;
  g_device_count = 0;
  g_initialized = 0;
  g_root_bus_id = DEVICE_BUS_ID_NONE;
  g_platform_bus_id = DEVICE_BUS_ID_NONE;
  g_hotplug_on_add = (device_hotplug_fn_t)0;
  g_hotplug_on_remove = (device_hotplug_fn_t)0;
  g_hotplug_ctx = (void *)0;
}

status_t device_bus_register_bus(const bus_t *bus_template, u64 *out_bus_id) {
  bus_t *dst;

  if (bus_template == (const bus_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_bus_count >= DEVICE_BUS_MAX_BUSES) {
    return STATUS_NO_MEMORY;
  }

  dst = &g_buses[g_bus_count];
  dst->id = DEVICE_BUS_ID_NONE;
  dst->parent_bus_id = bus_template->parent_bus_id;
  dst->type = bus_template->type;
  dst->name = bus_template->name;
  dst->id = g_bus_count;
  if (out_bus_id != (u64 *)0) {
    *out_bus_id = dst->id;
  }
  g_bus_count += 1ULL;
  return STATUS_OK;
}

status_t device_bus_register_device(const device_t *dev_template, u64 *out_device_id) {
  device_t *dst;
  u64 i;

  if (dev_template == (const device_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_device_count >= DEVICE_BUS_MAX_DEVICES) {
    return STATUS_NO_MEMORY;
  }

  dst = &g_devices[g_device_count];
  init_empty_device(dst);
  dst->parent_id = dev_template->parent_id;
  dst->bus_id = dev_template->bus_id;
  dst->name = dev_template->name;
  dst->class_id = dev_template->class_id;
  dst->vendor_id = dev_template->vendor_id;
  dst->device_id = dev_template->device_id;
  dst->revision = dev_template->revision;
  dst->class_code = dev_template->class_code;
  dst->subclass_code = dev_template->subclass_code;
  dst->prog_if = dev_template->prog_if;
  dst->resource_count = dev_template->resource_count;
  dst->driver_data = dev_template->driver_data;
  dst->active = 1;
  dst->id = g_device_count;
  if (dst->resource_count > DEVICE_BUS_MAX_RESOURCES) {
    dst->resource_count = DEVICE_BUS_MAX_RESOURCES;
  }
  for (i = 0; i < dst->resource_count; ++i) {
    dst->resources[i].kind = dev_template->resources[i].kind;
    dst->resources[i].base = dev_template->resources[i].base;
    dst->resources[i].size = dev_template->resources[i].size;
    dst->resources[i].flags = dev_template->resources[i].flags;
  }
  if (out_device_id != (u64 *)0) {
    *out_device_id = dst->id;
  }
  if (g_hotplug_on_add != (device_hotplug_fn_t)0) {
    g_hotplug_on_add(dst, g_hotplug_ctx);
  }
  g_device_count += 1ULL;
  return STATUS_OK;
}

status_t device_bus_remove_device(u64 device_id) {
  if (device_id >= g_device_count) {
    return STATUS_NOT_FOUND;
  }
  if (g_devices[device_id].active == 0) {
    return STATUS_NOT_FOUND;
  }
  g_devices[device_id].active = 0;
  if (g_hotplug_on_remove != (device_hotplug_fn_t)0) {
    g_hotplug_on_remove(&g_devices[device_id], g_hotplug_ctx);
  }
  return STATUS_OK;
}

status_t device_bus_register_hotplug(device_hotplug_fn_t on_add, device_hotplug_fn_t on_remove, void *ctx) {
  g_hotplug_on_add = on_add;
  g_hotplug_on_remove = on_remove;
  g_hotplug_ctx = ctx;
  return STATUS_OK;
}

status_t device_bus_replace_resources(u64 device_id, const device_resource_t *resources, u64 count) {
  device_t *dst;
  u64 i;

  if (device_id >= g_device_count) {
    return STATUS_NOT_FOUND;
  }
  if (count > 0 && resources == (const device_resource_t *)0) {
    return STATUS_INVALID_ARG;
  }

  dst = &g_devices[device_id];
  if (count > DEVICE_BUS_MAX_RESOURCES) {
    count = DEVICE_BUS_MAX_RESOURCES;
  }
  dst->resource_count = count;
  for (i = 0; i < DEVICE_BUS_MAX_RESOURCES; ++i) {
    if (i < count) {
      dst->resources[i] = resources[i];
    } else {
      dst->resources[i].kind = DEVICE_RESOURCE_NONE;
      dst->resources[i].base = 0;
      dst->resources[i].size = 0;
      dst->resources[i].flags = 0;
    }
  }
  return STATUS_OK;
}

const bus_t *device_bus_get_bus(u64 bus_id) {
  if (bus_id >= g_bus_count) {
    return (const bus_t *)0;
  }
  return &g_buses[bus_id];
}

const device_t *device_bus_get_device(u64 device_id) {
  if (device_id >= g_device_count) {
    return (const device_t *)0;
  }
  if (g_devices[device_id].active == 0) {
    return (const device_t *)0;
  }
  return &g_devices[device_id];
}

u64 device_bus_count(void) {
  u64 i;
  u64 count = 0;
  for (i = 0; i < g_device_count; ++i) {
    if (g_devices[i].active != 0) {
      count += 1ULL;
    }
  }
  return count;
}

const device_t *device_bus_device_at(u64 index) {
  u64 i;
  u64 active = 0;
  for (i = 0; i < g_device_count; ++i) {
    if (g_devices[i].active == 0) {
      continue;
    }
    if (active == index) {
      return &g_devices[i];
    }
    active += 1ULL;
  }
  return (const device_t *)0;
}

u64 device_bus_find_first_by_class(device_class_t class_id) {
  u64 i;
  for (i = 0; i < g_device_count; ++i) {
    if (g_devices[i].active != 0 && g_devices[i].class_id == class_id) {
      return g_devices[i].id;
    }
  }
  return DEVICE_BUS_ID_NONE;
}

u64 device_bus_find_next_by_class(device_class_t class_id, u64 after_id) {
  u64 i;
  for (i = after_id + 1ULL; i < g_device_count; ++i) {
    if (g_devices[i].active != 0 && g_devices[i].class_id == class_id) {
      return g_devices[i].id;
    }
  }
  return DEVICE_BUS_ID_NONE;
}

static status_t populate_platform_devices(const boot_info_t *boot_info, const hw_desc_t *hw) {
  u64 i;

  for (i = 0; i < hw->irq_controller_count; ++i) {
    device_t dev;
    init_empty_device(&dev);
    dev.id = DEVICE_BUS_ID_NONE;
    dev.parent_id = DEVICE_BUS_ID_NONE;
    dev.bus_id = g_platform_bus_id;
    dev.name = "irq-controller";
    dev.class_id = DEVICE_CLASS_IRQC;
    dev.vendor_id = 0;
    dev.device_id = hw->irq_controllers[i].type;
    dev.revision = 0;
    dev.class_code = 0;
    dev.subclass_code = 0;
    dev.prog_if = 0;
    dev.resource_count = 0;
    dev.driver_data = (void *)0;
    if (hw->irq_controllers[i].mmio_base != 0 || hw->irq_controllers[i].mmio_size != 0) {
      dev.resources[0].kind = DEVICE_RESOURCE_MMIO;
      dev.resources[0].base = hw->irq_controllers[i].mmio_base;
      dev.resources[0].size = hw->irq_controllers[i].mmio_size;
      dev.resources[0].flags = 0;
      dev.resource_count = 1;
    }
    if (device_bus_register_device(&dev, (u64 *)0) != STATUS_OK) {
      return STATUS_NO_MEMORY;
    }
  }

  for (i = 0; i < hw->timer_count; ++i) {
    device_t dev;
    init_empty_device(&dev);
    dev.id = DEVICE_BUS_ID_NONE;
    dev.parent_id = DEVICE_BUS_ID_NONE;
    dev.bus_id = g_platform_bus_id;
    dev.name = "timer";
    dev.class_id = DEVICE_CLASS_TIMER;
    dev.vendor_id = 0;
    dev.device_id = 0;
    dev.revision = 0;
    dev.class_code = 0;
    dev.subclass_code = 0;
    dev.prog_if = 0;
    dev.resource_count = 0;
    dev.driver_data = (void *)0;
    if (hw->timers[i].mmio_base != 0 || hw->timers[i].mmio_size != 0) {
      dev.resources[0].kind = DEVICE_RESOURCE_MMIO;
      dev.resources[0].base = hw->timers[i].mmio_base;
      dev.resources[0].size = hw->timers[i].mmio_size;
      dev.resources[0].flags = 0;
      dev.resource_count = 1;
    }
    if (hw->timers[i].irq != 0 && dev.resource_count < DEVICE_BUS_MAX_RESOURCES) {
      dev.resources[dev.resource_count].kind = DEVICE_RESOURCE_IRQ;
      dev.resources[dev.resource_count].base = hw->timers[i].irq;
      dev.resources[dev.resource_count].size = 1;
      dev.resources[dev.resource_count].flags = 0;
      dev.resource_count += 1;
    }
    if (device_bus_register_device(&dev, (u64 *)0) != STATUS_OK) {
      return STATUS_NO_MEMORY;
    }
  }

  for (i = 0; i < hw->uart_count; ++i) {
    device_t dev;
    init_empty_device(&dev);
    dev.id = DEVICE_BUS_ID_NONE;
    dev.parent_id = DEVICE_BUS_ID_NONE;
    dev.bus_id = g_platform_bus_id;
    dev.name = "uart";
    dev.class_id = DEVICE_CLASS_CONSOLE;
    dev.vendor_id = 0;
    dev.device_id = 0;
    dev.revision = 0;
    dev.class_code = 0;
    dev.subclass_code = 0;
    dev.prog_if = 0;
    dev.resource_count = 1;
    dev.driver_data = (void *)0;
    dev.resources[0].kind = DEVICE_RESOURCE_MMIO;
    dev.resources[0].base = hw->uarts[i].base;
    dev.resources[0].size = 0x1000ULL;
    dev.resources[0].flags = 0;
    if (hw->uarts[i].irq != 0) {
      dev.resources[1].kind = DEVICE_RESOURCE_IRQ;
      dev.resources[1].base = hw->uarts[i].irq;
      dev.resources[1].size = 1;
      dev.resources[1].flags = 0;
      dev.resource_count = 2;
    }
    if (device_bus_register_device(&dev, (u64 *)0) != STATUS_OK) {
      return STATUS_NO_MEMORY;
    }
  }

  for (i = 0; i < hw->mmio_region_count; ++i) {
    device_t dev;
    init_empty_device(&dev);
    dev.id = DEVICE_BUS_ID_NONE;
    dev.parent_id = DEVICE_BUS_ID_NONE;
    dev.bus_id = g_platform_bus_id;
    dev.name = "mmio-region";
    dev.class_id = DEVICE_CLASS_MMIO;
    dev.vendor_id = 0;
    dev.device_id = 0;
    dev.revision = 0;
    dev.class_code = 0;
    dev.subclass_code = 0;
    dev.prog_if = 0;
    dev.resource_count = 1;
    dev.driver_data = (void *)0;
    dev.resources[0].kind = DEVICE_RESOURCE_MMIO;
    dev.resources[0].base = hw->mmio_regions[i].base;
    dev.resources[0].size = hw->mmio_regions[i].size;
    dev.resources[0].flags = 0;
    if (device_bus_register_device(&dev, (u64 *)0) != STATUS_OK) {
      return STATUS_NO_MEMORY;
    }
  }

  if ((boot_info->valid_mask & BOOT_INFO_HAS_FRAMEBUFFER) != 0ULL) {
    device_t dev;
    init_empty_device(&dev);
    dev.id = DEVICE_BUS_ID_NONE;
    dev.parent_id = DEVICE_BUS_ID_NONE;
    dev.bus_id = g_platform_bus_id;
    dev.name = "framebuffer";
    dev.class_id = DEVICE_CLASS_FRAMEBUFFER;
    dev.vendor_id = 0;
    dev.device_id = boot_info->framebuffer_format;
    dev.revision = 0;
    dev.class_code = 0;
    dev.subclass_code = 0;
    dev.prog_if = 0;
    dev.resource_count = 1;
    dev.driver_data = (void *)0;
    dev.resources[0].kind = DEVICE_RESOURCE_MMIO;
    dev.resources[0].base = boot_info->framebuffer_base;
    dev.resources[0].size = (u64)boot_info->framebuffer_pixels_per_scanline * (u64)boot_info->framebuffer_height *
                            4ULL;
    dev.resources[0].flags = 0;
    if (device_bus_register_device(&dev, (u64 *)0) != STATUS_OK) {
      return STATUS_NO_MEMORY;
    }
  }

  return STATUS_OK;
}

status_t device_bus_init(const boot_info_t *boot_info, const hw_desc_t *hw) {
  bus_t root_bus;
  bus_t platform_bus;
  status_t st;

  if (boot_info == (const boot_info_t *)0 || hw == (const hw_desc_t *)0) {
    return STATUS_INVALID_ARG;
  }

  device_bus_reset();

  root_bus.id = DEVICE_BUS_ID_NONE;
  root_bus.parent_bus_id = DEVICE_BUS_ID_NONE;
  root_bus.type = BUS_TYPE_ROOT;
  root_bus.name = "root";
  st = device_bus_register_bus(&root_bus, &g_root_bus_id);
  if (st != STATUS_OK) {
    return st;
  }

  platform_bus.id = DEVICE_BUS_ID_NONE;
  platform_bus.parent_bus_id = g_root_bus_id;
  platform_bus.type = BUS_TYPE_PLATFORM;
  platform_bus.name = "platform";
  st = device_bus_register_bus(&platform_bus, &g_platform_bus_id);
  if (st != STATUS_OK) {
    return st;
  }

  st = populate_platform_devices(boot_info, hw);
  if (st != STATUS_OK) {
    return st;
  }

  g_initialized = 1ULL;
  return STATUS_OK;
}

void device_bus_dump(void) {
  u64 i;

  if (g_initialized == 0ULL) {
    kprintf("device-bus: not initialized\n");
    return;
  }

  kprintf("device-bus: buses=%llu devices=%llu\n", g_bus_count, g_device_count);
  for (i = 0; i < g_bus_count; ++i) {
    const bus_t *b = &g_buses[i];
    kprintf("  bus[%llu]: type=%s parent=%llu name=%s\n", b->id, bus_name(b->type), b->parent_bus_id,
            b->name == (const char *)0 ? "<none>" : b->name);
  }
  for (i = 0; i < g_device_count; ++i) {
    const device_t *d = &g_devices[i];
    kprintf("  dev[%llu]: bus=%llu class=%s name=%s resources=%llu\n", d->id, d->bus_id, class_name(d->class_id),
            d->name == (const char *)0 ? "<none>" : d->name, d->resource_count);
  }
}

static hw_resource_type_t to_hw_type(device_resource_kind_t kind) {
  switch (kind) {
  case DEVICE_RESOURCE_MMIO:
    return HW_RESOURCE_MMIO;
  case DEVICE_RESOURCE_IRQ:
    return HW_RESOURCE_IRQ;
  case DEVICE_RESOURCE_IOPORT:
    return HW_RESOURCE_IOPORT;
  case DEVICE_RESOURCE_DMA:
    return HW_RESOURCE_DMA;
  default:
    return HW_RESOURCE_NONE;
  }
}

static device_resource_kind_t to_device_kind(hw_resource_type_t type) {
  switch (type) {
  case HW_RESOURCE_MMIO:
    return DEVICE_RESOURCE_MMIO;
  case HW_RESOURCE_IRQ:
    return DEVICE_RESOURCE_IRQ;
  case HW_RESOURCE_IOPORT:
    return DEVICE_RESOURCE_IOPORT;
  case HW_RESOURCE_DMA:
    return DEVICE_RESOURCE_DMA;
  default:
    return DEVICE_RESOURCE_NONE;
  }
}

status_t hw_resource_init(const boot_info_t *boot_info) {
  (void)boot_info;
  return g_initialized ? STATUS_OK : STATUS_DEFERRED;
}

status_t hw_resource_attach(u64 device_id, const hw_resource_t *list, u64 count) {
  device_resource_t resources[DEVICE_BUS_MAX_RESOURCES];
  u64 i;

  if (count > 0 && list == (const hw_resource_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (count > DEVICE_BUS_MAX_RESOURCES) {
    count = DEVICE_BUS_MAX_RESOURCES;
  }
  for (i = 0; i < count; ++i) {
    resources[i].kind = to_device_kind(list[i].type);
    resources[i].base = list[i].base;
    resources[i].size = list[i].size;
    resources[i].flags = list[i].flags;
  }
  return device_bus_replace_resources(device_id, resources, count);
}

status_t hw_resource_get(u64 device_id, hw_resource_type_t type, u64 index, hw_resource_t *out) {
  const device_t *dev;
  u64 i;
  u64 found = 0;

  if (out == (hw_resource_t *)0) {
    return STATUS_INVALID_ARG;
  }
  dev = device_bus_get_device(device_id);
  if (dev == (const device_t *)0) {
    return STATUS_NOT_FOUND;
  }

  for (i = 0; i < dev->resource_count; ++i) {
    hw_resource_type_t cur = to_hw_type(dev->resources[i].kind);
    if (type != HW_RESOURCE_NONE && cur != type) {
      continue;
    }
    if (found == index) {
      out->type = cur;
      out->base = dev->resources[i].base;
      out->size = dev->resources[i].size;
      out->flags = dev->resources[i].flags;
      return STATUS_OK;
    }
    found += 1ULL;
  }
  return STATUS_NOT_FOUND;
}

u64 hw_resource_count(u64 device_id, hw_resource_type_t type) {
  const device_t *dev;
  u64 i;
  u64 count = 0;

  dev = device_bus_get_device(device_id);
  if (dev == (const device_t *)0) {
    return 0;
  }
  for (i = 0; i < dev->resource_count; ++i) {
    if (type == HW_RESOURCE_NONE || to_hw_type(dev->resources[i].kind) == type) {
      count += 1ULL;
    }
  }
  return count;
}

status_t hw_resource_view(u64 device_id, device_resource_view_t *out) {
  const device_t *dev;

  if (out == (device_resource_view_t *)0) {
    return STATUS_INVALID_ARG;
  }
  dev = device_bus_get_device(device_id);
  if (dev == (const device_t *)0) {
    return STATUS_NOT_FOUND;
  }
  out->device_id = device_id;
  out->total_count = dev->resource_count;
  return STATUS_OK;
}
