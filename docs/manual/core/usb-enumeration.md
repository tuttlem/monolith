# USB Enumeration Skeleton

The USB layer adds baseline host-controller discovery and root-hub placeholders into the generic device graph.

## Public API

Header: `kernel/include/usb.h`

- `status_t usb_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 usb_host_count(void)`
- `BOOT_U64 usb_device_count(void)`

## Current Behavior

- Scans existing `DEVICE_CLASS_PCI_DEVICE` entries.
- Matches USB controller class code (`0x0c` / subclass `0x03`).
- For each match:
  - registers `DEVICE_CLASS_USB_HOST` node
  - registers one `DEVICE_CLASS_USB_DEVICE` root-hub placeholder node

## Scope Notes

- This phase is structural enumeration only.
- It does not yet implement full transfer scheduling, endpoint setup, or class drivers.
