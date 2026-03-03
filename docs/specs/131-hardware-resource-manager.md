# 131 Hardware Resource Manager

## Goal

Provide one architecture-neutral resource contract for device-facing hardware assets so OS developers do not parse raw ACPI/DT/PnP details in early bring-up.

## Why This Exists

Current discovery and device model expose topology, but resource handling still leaks backend details. A resource manager allows drivers and OS layers to consume IRQ/MMIO/DMA/clock/reset resources through one interface.

## In Scope

- Canonical resource descriptor type:
  - IRQ lines and vectors
  - MMIO ranges
  - PIO ranges (where architecture supports it)
  - DMA apertures/constraints
  - clock handles
  - reset handles
- Per-device resource tables attached during discovery/model population.
- Query API from generic kernel and future OS layers.
- Resource flags (`shared`, `edge/level`, `secure`, `cacheable`, etc.) as metadata only.

## Out of Scope

- Device policy (probe order policy beyond deterministic default).
- Power management policy.
- Full driver model replacement.

## Public Interfaces

- Header: `kernel/include/hw_resource.h`
- Core types:
  - `resource_type_t`
  - `resource_flags_t`
  - `hw_resource_t`
  - `device_resource_view_t`
- APIs:
  - `status_t hw_resource_init(const boot_info_t *boot_info)`
  - `status_t hw_resource_attach(device_id_t dev, const hw_resource_t *list, BOOT_U64 count)`
  - `status_t hw_resource_get(device_id_t dev, resource_type_t type, BOOT_U64 index, hw_resource_t *out)`
  - `BOOT_U64 hw_resource_count(device_id_t dev, resource_type_t type)`

## Architecture Backends

- `x86_64`
  - ACPI/PCI-derived IRQ and BAR resources.
  - PIO resources exposed where legacy devices require it.
- `arm64`
  - ACPI `_CRS` or DT `reg/interrupts/clocks/resets` translation.
- `riscv64`
  - DT-centric translation (`reg`, interrupt parents/specifiers, optional clocks/resets).

## Init Order

1. `hw_discovery_init`
2. `device_bus_init`
3. `hw_resource_init`
4. domain enumeration and driver probe

## Diagnostics

- Resource dump per device in `device-report` path.
- One-line backend source marker (`acpi`, `dt`, `mixed`).

## Tests

- Unit tests for descriptor validation and flag normalization.
- Boot smoke checks:
  - no invalid ranges
  - no overlapping MMIO per single device
  - deterministic counts per class.

## Acceptance Criteria

1. Every discovered non-virtual device can expose zero or more `hw_resource_t` records.
2. No architecture-specific resource parsing in generic drivers.
3. `STATUS_DEFERRED` is allowed only for truly absent optional resource classes.

## Risks

- Early overfitting to one firmware source.
- Mitigation: normalize from both ACPI/DT into the same internal table.
