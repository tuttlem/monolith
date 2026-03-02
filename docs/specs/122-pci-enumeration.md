# 122 PCI/PCIe Enumeration

## Goal

Provide baseline PCI/PCIe enumeration and resource extraction so OS writers can see and target real platform devices.

## Scope

- Enumerate buses/devices/functions.
- Parse and expose:
  - vendor/device IDs
  - class/subclass/prog-if
  - BAR resources (MMIO/PIO)
  - interrupt metadata (legacy pin/line, MSI capability presence where feasible)
- Populate device graph nodes as `bus=pci` children.
- Add deterministic probe order and readable diagnostics.

## Non-Goals

- Full MSI/MSI-X programming in this phase.
- SR-IOV, ACS, ATS, advanced PCIe services.

## Architecture Notes

- x86_64:
  - start with ECAM where available, fallback path as needed.
  - ACPI integration for host bridge ranges and routing hints.
- arm64/riscv64:
  - use DT/ACPI host bridge description paths.

## Acceptance Criteria

- At least one QEMU-backed platform shows enumerated PCI devices in kernel logs.
- BARs and class IDs are exposed in device descriptors.
- Drivers can match on vendor/device/class keys.
