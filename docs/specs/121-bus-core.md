# 121 Bus Core and Device Graph

## Goal

Introduce a generic bus/device substrate that represents discovered hardware as a stable graph usable by any future OS personality.

## Scope

- Add core device graph types:
  - `bus_type_t`
  - `device_t`
  - `device_resource_t` (MMIO/PIO/IRQ/DMA)
- Add lifecycle primitives:
  - create/register device
  - parent/child attachment
  - enumerate children
- Add driver matching contracts independent of OS policy:
  - match keys and scoring
  - probe/attach hooks
- Keep this phase static and early-boot safe (no dynamic module loading required).

## Non-Goals

- Full hotplug orchestration.
- Userspace-facing device manager.
- Power management runtime policy.

## API Targets

- New core headers for bus/device/resource abstractions.
- Deterministic registration and attach order.
- Structured diagnostic dump of device tree/graph.

## Architecture Coverage

- x86_64: integrate with existing ACPI/PCI discovery inputs.
- arm64: integrate with ACPI and DT discovery paths.
- riscv64: integrate with DT discovery path.

## Acceptance Criteria

- Kernel can produce a stable device graph even before full bus enumerators are complete.
- Existing `120-device-model` can bind to `device_t` instances instead of ad-hoc node shims.
- Graph output is architecture-agnostic and reproducible across runs.
