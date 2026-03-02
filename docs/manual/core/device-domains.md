# Device Domain Baseline (Block/Input/Display)

This layer creates baseline domain-class devices from enumerated hardware so downstream kernels can consume common classes without bus-specific logic.

## Public API

Header: `kernel/include/device_domains.h`

- `status_t device_domains_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 block_device_count(void)`
- `BOOT_U64 input_device_count(void)`
- `BOOT_U64 display_device_count(void)`

## Current Mapping Rules

`device_domains_enumerate()` scans `device_bus` and creates derived domain devices:

- Block:
  - PCI class `0x01`
  - produced as `DEVICE_CLASS_BLOCK`
- Display:
  - PCI class `0x03`
  - framebuffer devices
  - produced as `DEVICE_CLASS_DISPLAY`
- Input:
  - PCI class `0x09`
  - USB device class `0x03` (HID-style placeholder path)
  - produced as `DEVICE_CLASS_INPUT`

## Notes

- This is a baseline classification layer, not a full I/O stack.
- Domain devices keep parent linkage to source bus devices for traceability.
