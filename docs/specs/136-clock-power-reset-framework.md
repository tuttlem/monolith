# 136 Clock, Power, and Reset Framework

## Goal

Provide a minimal control-plane substrate for SoC/peripheral clock and reset operations, without enforcing power policy.

## In Scope

- Clock handle discovery and control:
  - enable/disable
  - set/get rate (best-effort)
- Reset control operations:
  - assert/deassert/pulse
- Optional power-domain hooks:
  - on/off/status only.

## Out of Scope

- Runtime PM governor policy.
- DVFS policy.

## Public Interfaces

- Headers: `kernel/include/clock.h`, `kernel/include/reset.h`, `kernel/include/power_domain.h`
- APIs:
  - `status_t clock_enable(clock_id_t clk)`
  - `status_t clock_set_rate(clock_id_t clk, u64 hz)`
  - `status_t reset_deassert(reset_id_t rst)`

## Architecture Notes

- Primarily meaningful for arm64/riscv64 platform devices.
- x86_64 backend can be minimal/no-op for unsupported controls.

## Tests

- Mock backend unit tests for control sequencing.
- Platform-device init paths using clock/reset framework.

## Acceptance Criteria

1. No platform driver in generic path touches raw clock/reset registers directly.
2. Unsupported controls return explicit `STATUS_NOT_SUPPORTED`.
3. Discovery can attach clock/reset resources to device records.
