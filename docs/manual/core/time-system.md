# Time System

API: `kernel/include/timebase.h`  
Implementation: `kernel/timebase.c`

## Model

The time subsystem combines:
- `clocksource`: free-running cycle counter + frequency
- `clockevent`: interrupt event source used for periodic ticks

## Public API

- `status_t time_init(const boot_info_t *boot_info)`
- `u64 time_now_ns(void)`
- `u64 time_ticks(void)`
- `u64 time_hz(void)`
- `u64 time_cycles_to_ns(u64 cycles)`
- `u64 time_ns_to_cycles(u64 ns)`
- `status_t time_quality(time_quality_t *out)`
- `const clocksource_t *time_clocksource(void)`
- `const clockevent_t *time_clockevent(void)`

Compatibility shim:
- `kernel/timer.c` keeps `timer_init`, `timer_ticks`, `timer_hz`

## Initialization Flow

1. call `arch_timer_init` to obtain timer frequency and vector
2. register timer IRQ handler in interrupt core
3. map vector to IRQ through controller layer
4. enable timer IRQ where backend supports it
5. choose clocksource:
   - preferred: architecture cycle counter (`arch_cycle_counter`)
   - fallback: tick-derived time

## Monotonicity Rules

- `time_now_ns` is clamped to never move backwards
- tick count can be refreshed from cycle-derived nanoseconds for consistency
- overflow-safe conversion uses saturating mul/div helpers

## Quality Reporting

`time_quality_t` reports current calibration quality:
- `stable`: time source is treated as stable enough for scheduler/runtime use.
- `unstable`: backend quality is conservative and should be treated as lower confidence.
- `emulated`: source is likely virtualized/emulated or fallback-derived.
- `calibrated_hz`: active source frequency in Hz.
- `drift_ppm_bound`: estimated drift bound in parts-per-million.
- `cross_cpu_checked`: whether cross-CPU monotonic probe was attempted.
- `cross_cpu_passed`: result of cross-CPU monotonic probe.

## Architecture Hook

`arch_timer_clocksource_hz(const boot_info_t *boot_info)` exposes cycle-counter frequency for monotonic conversion.
