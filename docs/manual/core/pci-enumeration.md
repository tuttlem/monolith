# PCI Enumeration

The PCI layer provides baseline PCI/PCIe discovery and injects PCI devices into the generic device graph.

## Public API

Header: `kernel/include/pci.h`

- `status_t pci_enumerate(const boot_info_t *boot_info)`
- `u64 pci_device_count(void)`

## Current Backend Status

- `x86_64`: implemented using legacy PCI config mechanism I/O ports (`0xCF8/0xCFC`).
- `arm64`: API is active and returns concrete completion status; device count may legitimately be zero on platforms without a wired PCI scan backend.
- `riscv64`: API is active and returns concrete completion status; device count may legitimately be zero on platforms without a wired PCI scan backend.

## x86_64 Behavior

- Scans buses `0..255`, slots `0..31`, functions `0..7` (multifunction-aware).
- Reads vendor/device/class/subclass/prog-if/revision from config space.
- Reads BAR registers and exposes them as `DEVICE_RESOURCE_MMIO`/`DEVICE_RESOURCE_IOPORT` resources.
- Registers each discovered function as `DEVICE_CLASS_PCI_DEVICE` in the bus graph.

## Integration Point

`kmain` runs `pci_enumerate()` after `device_bus_init()`, so higher layers can consume PCI devices from `device_bus`.

## Notes

- This phase focuses on deterministic discovery and reporting.
- Advanced PCI services (MSI/MSI-X programming, SR-IOV, power management) are intentionally deferred.
