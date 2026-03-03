#include "usb.h"
#include "print.h"

static BOOT_U64 g_usb_bus_id = DEVICE_BUS_ID_NONE;
static BOOT_U64 g_usb_host_count;
static BOOT_U64 g_usb_device_count;

static status_t ensure_usb_bus(void) {
  bus_t bus;

  if (g_usb_bus_id != DEVICE_BUS_ID_NONE) {
    return STATUS_OK;
  }

  bus.id = DEVICE_BUS_ID_NONE;
  bus.parent_bus_id = 0;
  bus.type = BUS_TYPE_USB;
  bus.name = "usb";
  return device_bus_register_bus(&bus, &g_usb_bus_id);
}

status_t usb_enumerate(const boot_info_t *boot_info) {
  BOOT_U64 i;
  status_t st;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  st = ensure_usb_bus();
  if (st != STATUS_OK) {
    return st;
  }

  g_usb_host_count = 0;
  g_usb_device_count = 0;

  for (i = 0; i < device_bus_count(); ++i) {
    const device_t *src = device_bus_device_at(i);

    if (src == (const device_t *)0 || src->class_id != DEVICE_CLASS_PCI_DEVICE) {
      continue;
    }
    if (src->class_code != 0x0cULL || src->subclass_code != 0x03ULL) {
      continue;
    }

    {
      device_t host;
      BOOT_U64 host_id = DEVICE_BUS_ID_NONE;
      BOOT_U64 r;

      host.id = DEVICE_BUS_ID_NONE;
      host.parent_id = src->id;
      host.bus_id = g_usb_bus_id;
      host.name = "usb-host";
      host.class_id = DEVICE_CLASS_USB_HOST;
      host.vendor_id = src->vendor_id;
      host.device_id = src->device_id;
      host.revision = src->revision;
      host.class_code = src->class_code;
      host.subclass_code = src->subclass_code;
      host.prog_if = src->prog_if;
      host.resource_count = src->resource_count;
      host.driver_data = (void *)0;
      for (r = 0; r < DEVICE_BUS_MAX_RESOURCES; ++r) {
        host.resources[r].kind = src->resources[r].kind;
        host.resources[r].base = src->resources[r].base;
        host.resources[r].size = src->resources[r].size;
        host.resources[r].flags = src->resources[r].flags;
      }

      st = device_bus_register_device(&host, &host_id);
      if (st != STATUS_OK) {
        return st;
      }
      g_usb_host_count += 1ULL;

      {
        device_t root_hub;
        BOOT_U64 rr;

        root_hub.id = DEVICE_BUS_ID_NONE;
        root_hub.parent_id = host_id;
        root_hub.bus_id = g_usb_bus_id;
        root_hub.name = "usb-root-hub";
        root_hub.class_id = DEVICE_CLASS_USB_DEVICE;
        root_hub.vendor_id = 0;
        root_hub.device_id = 0;
        root_hub.revision = 0;
        root_hub.class_code = 0x09ULL;
        root_hub.subclass_code = 0;
        root_hub.prog_if = 0;
        root_hub.resource_count = 0;
        root_hub.driver_data = (void *)0;
        for (rr = 0; rr < DEVICE_BUS_MAX_RESOURCES; ++rr) {
          root_hub.resources[rr].kind = DEVICE_RESOURCE_NONE;
          root_hub.resources[rr].base = 0;
          root_hub.resources[rr].size = 0;
          root_hub.resources[rr].flags = 0;
        }

        st = device_bus_register_device(&root_hub, (BOOT_U64 *)0);
        if (st != STATUS_OK) {
          return st;
        }
        g_usb_device_count += 1ULL;
      }
    }
  }

  if (g_usb_host_count == 0ULL) {
    return STATUS_DEFERRED;
  }

  kprintf("usb: hosts=%llu devices=%llu\n", g_usb_host_count, g_usb_device_count);
  return STATUS_OK;
}

BOOT_U64 usb_host_count(void) { return g_usb_host_count; }
BOOT_U64 usb_device_count(void) { return g_usb_device_count; }
