# Clock, Power, and Reset Framework

This layer provides generic control-plane APIs for platform device bring-up.

## Purpose

- Keep clock/reset/power control calls architecture-neutral.
- Allow platform drivers to use one stable API.
- Keep policy (runtime PM, DVFS) outside substrate.

## Public API

Headers:
- `kernel/include/clock.h`
- `kernel/include/reset.h`
- `kernel/include/power_domain.h`

APIs:
- `clock_enable`, `clock_disable`, `clock_set_rate`, `clock_get_rate`
- `reset_assert`, `reset_deassert`, `reset_pulse`
- `power_domain_on`, `power_domain_off`, `power_domain_status`

## Notes

- Baseline implementation provides deterministic status semantics.
- Hardware-specific backend wiring can be added later without API change.
