#include "device_report.h"
#include "print.h"

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

BOOT_U64 device_report_count(void) { return device_bus_count(); }

status_t device_report_get(BOOT_U64 index, device_report_entry_t *out_entry) {
  const device_t *d;

  if (out_entry == (device_report_entry_t *)0) {
    return STATUS_INVALID_ARG;
  }

  d = device_bus_device_at(index);
  if (d == (const device_t *)0) {
    return STATUS_NOT_FOUND;
  }

  out_entry->id = d->id;
  out_entry->parent_id = d->parent_id;
  out_entry->bus_id = d->bus_id;
  out_entry->name = d->name;
  out_entry->class_id = d->class_id;
  out_entry->vendor_id = d->vendor_id;
  out_entry->device_id = d->device_id;
  out_entry->class_code = d->class_code;
  out_entry->subclass_code = d->subclass_code;
  out_entry->prog_if = d->prog_if;
  out_entry->resource_count = d->resource_count;
  return STATUS_OK;
}

void device_report_dump_all(void) {
  BOOT_U64 i;
  kprintf("device-report: total=%llu\n", device_report_count());
  for (i = 0; i < device_report_count(); ++i) {
    const device_t *d = device_bus_device_at(i);
    if (d == (const device_t *)0) {
      continue;
    }
    kprintf("  dev[%llu]: class=%s bus=%llu parent=%llu ven=0x%llx dev=0x%llx cc=%02llx/%02llx/%02llx res=%llu name=%s\n",
            d->id, class_name(d->class_id), d->bus_id, d->parent_id, d->vendor_id, d->device_id, d->class_code,
            d->subclass_code, d->prog_if, d->resource_count, d->name == (const char *)0 ? "<none>" : d->name);
  }
}

void device_report_dump_class(device_class_t class_id) {
  BOOT_U64 id = device_bus_find_first_by_class(class_id);
  kprintf("device-report: class=%s\n", class_name(class_id));
  while (id != DEVICE_BUS_ID_NONE) {
    const device_t *d = device_bus_get_device(id);
    if (d != (const device_t *)0) {
      kprintf("  dev[%llu]: bus=%llu parent=%llu ven=0x%llx dev=0x%llx res=%llu name=%s\n", d->id, d->bus_id,
              d->parent_id, d->vendor_id, d->device_id, d->resource_count,
              d->name == (const char *)0 ? "<none>" : d->name);
    }
    id = device_bus_find_next_by_class(class_id, id);
  }
}
