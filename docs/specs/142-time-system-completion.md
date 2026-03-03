# 142 Time System Completion

## Goal

Finish time substrate so monotonic time is stable, calibrated, and consistent across CPUs and architectures.

## In Scope

- Complete `clocksource` and `clockevent` split from spec `070`.
- Calibration quality flags (`stable`, `unstable`, `emulated`).
- Cross-CPU monotonicity checks.
- Conversion helpers (`cycles <-> ns`) with bounded drift reporting.

## Out of Scope

- Wall-clock/timezone/RTC policy.

## Public Interfaces

- Header: `kernel/include/timebase.h`
- APIs:
  - `BOOT_U64 time_now_ns(void)`
  - `BOOT_U64 time_ticks(void)`
  - `BOOT_U64 time_hz(void)`
  - `status_t time_quality(time_quality_t *out)`

## Architecture Backends

- `x86_64`: TSC primary with PIT/HPET/APIC fallback path.
- `arm64`: generic counter primary (`CNTVCT/CNTFRQ`).
- `riscv64`: `time/timeh` or SBI time source with quality tagging.

## Tests

- monotonic progression tests.
- drift bounds during fixed interval self-test.
- timer interrupt jitter summary in diagnostics.

## Acceptance Criteria

1. `time_now_ns` monotonic on all architectures.
2. Quality status reported at boot.
3. No architecture reports false "stable" quality when emulated/uncertain.
