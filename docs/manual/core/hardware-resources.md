# Hardware Resource Manager

This layer provides an architecture-neutral view of device resources on top of the bus/device graph.

## Purpose

- Expose MMIO/IRQ/IOPORT/DMA resources through one stable API.
- Let generic code query resources without parsing architecture firmware formats.
- Allow controlled resource replacement during bring-up diagnostics.

## Public API

Header: `kernel/include/hw_resource.h`

- `status_t hw_resource_init(const boot_info_t *boot_info)`
- `status_t hw_resource_attach(BOOT_U64 device_id, const hw_resource_t *list, BOOT_U64 count)`
- `status_t hw_resource_get(BOOT_U64 device_id, hw_resource_type_t type, BOOT_U64 index, hw_resource_t *out)`
- `BOOT_U64 hw_resource_count(BOOT_U64 device_id, hw_resource_type_t type)`
- `status_t hw_resource_view(BOOT_U64 device_id, device_resource_view_t *out)`

## Notes

- Current implementation normalizes existing `device_t.resources[]` entries.
- `HW_RESOURCE_CLOCK` and `HW_RESOURCE_RESET` are reserved for future resource providers.
