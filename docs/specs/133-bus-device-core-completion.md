# 133 Bus and Device Core Completion

## Goal

Complete bus-level substrate so device enumeration is consistent and extensible, while remaining OS-policy neutral.

## In Scope

- Stable bus types: `root`, `platform`, `pci`, `usb`, `virtio`.
- Parent/child graph consistency rules.
- Device identity contract:
  - vendor/device ids
  - class/subclass/progif
  - compatible strings/UID where relevant.
- Hotplug event skeleton (add/remove callbacks).
- Deterministic probe ordering rules.

## Out of Scope

- Full hotplug recovery/policy.
- Driver userspace model.

## Public Interfaces

- Header: `kernel/include/device_bus.h`, `kernel/include/device_model.h`
- APIs:
  - `status_t device_bus_register_type(...)`
  - `status_t device_add(...)`
  - `status_t device_remove(...)`
  - `status_t device_enumerate_children(device_id_t parent, ...)`
  - `status_t driver_bind(device_id_t dev, driver_id_t drv)`

## Architecture Notes

- Architecture-independent core, architecture-specific enumerators feed canonical `device_t` records.

## Tests

- Graph integrity tests (no cycles, valid parent pointers).
- Class/domain counts remain deterministic across repeated boots.
- Probe ordering test fixture.

## Acceptance Criteria

1. All discovered hardware appears in one canonical device graph.
2. Device graph can be consumed without architecture conditionals.
3. Hotplug event APIs compile and run no-op flows even when backend lacks support.
