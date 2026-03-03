# Device Bus Core

The bus core provides a stable hardware graph API above firmware discovery and below driver binding.

## Purpose

- Normalize discovered hardware into `bus_t` and `device_t` objects.
- Keep driver matching independent from ACPI/DT/raw tables.
- Provide deterministic enumeration order for early bring-up.

## Public API

Header: `kernel/include/device_bus.h`

- `status_t device_bus_init(const boot_info_t *boot_info, const hw_desc_t *hw)`
- `void device_bus_reset(void)`
- `status_t device_bus_register_bus(const bus_t *bus_template, BOOT_U64 *out_bus_id)`
- `status_t device_bus_register_device(const device_t *dev_template, BOOT_U64 *out_device_id)`
- `status_t device_bus_remove_device(BOOT_U64 device_id)`
- `status_t device_bus_register_hotplug(device_hotplug_fn_t on_add, device_hotplug_fn_t on_remove, void *ctx)`
- `const bus_t *device_bus_get_bus(BOOT_U64 bus_id)`
- `const device_t *device_bus_get_device(BOOT_U64 device_id)`
- `BOOT_U64 device_bus_count(void)`
- `const device_t *device_bus_device_at(BOOT_U64 index)`
- `BOOT_U64 device_bus_find_first_by_class(device_class_t class_id)`
- `BOOT_U64 device_bus_find_next_by_class(device_class_t class_id, BOOT_U64 after_id)`
- `void device_bus_dump(void)`

## Current Graph Population

`device_bus_init()` currently builds:
- root bus
- platform bus
- platform devices derived from `hw_desc_t`:
  - IRQ controllers
  - timers
  - UART/console devices
  - MMIO regions
  - framebuffer (when present)

## Driver Model Integration

`device_model.c` now probes against bus `device_t` nodes, not direct `hw_desc` arrays.

This is the transition point that enables future bus-specific enumerators (PCI/USB) to feed the same generic driver workflow.

## Completion Notes

- `BUS_TYPE_VIRTIO` is now part of the canonical bus enum for future transport enumeration.
- Devices carry an `active` marker so remove events can invalidate graph entries without ID reuse.
- Optional hotplug callbacks provide add/remove notifications while keeping policy outside this layer.
