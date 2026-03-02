# 070 Time System: Clocksource + Clockevent

## Goal

Move from raw periodic tick to a generic time subsystem with monotonic time and pluggable event source.

## Concepts

- `clocksource`: free-running counter + frequency
- `clockevent`: programmable interrupt event source (periodic or oneshot)

## API Surface (Target)

```c
typedef struct {
  const char *name;
  BOOT_U64 freq_hz;
  BOOT_U64 (*read_cycles)(void);
} clocksource_t;

typedef struct {
  const char *name;
  status_t (*set_periodic)(BOOT_U64 hz);
  status_t (*set_oneshot_ns)(BOOT_U64 delta_ns);
  void (*ack)(void);
} clockevent_t;

status_t time_init(const boot_info_t *boot_info);
BOOT_U64 time_now_ns(void);
BOOT_U64 time_ticks(void);
```

## Requirements

1. monotonic `time_now_ns()` available once `time_init` succeeds.
2. current `timer_ticks()` compatibility shim can remain temporarily.
3. conversion must avoid 64-bit overflow hazards.
4. time read path should be lockless on uniprocessor bring-up.

## Architecture Targets

### x86_64
- clocksource: TSC (with fallback if unstable)
- clockevent: LAPIC timer preferred, PIT fallback path for bring-up

### arm64
- clocksource: generic counter (`CNTVCT_EL0`)
- clockevent: generic timer

### riscv64
- clocksource: `time` CSR or platform time source
- clockevent: SBI timer or platform local timer

## Acceptance Criteria

- `time_now_ns()` returns increasing values on all architectures
- timer interrupt path is validated by self-test when backend exists
- riscv64 leaves deferred state only if platform truly lacks timer support

## Implementation Notes (Current Repository)

`070-time-system` is implemented with a new generic timebase layer:
- `kernel/include/timebase.h`
- `kernel/timebase.c`

Provided APIs:
- `time_init(const boot_info_t *boot_info)`
- `time_now_ns(void)`
- `time_ticks(void)`
- `time_hz(void)`
- clocksource/clockevent descriptors via `time_clocksource()` / `time_clockevent()`

Compatibility:
- existing `timer_*` API is preserved in `kernel/timer.c` as a shim to `timebase`

Clocksource model:
- preferred: architecture cycle counter + `arch_timer_clocksource_hz()`
- fallback: tick-derived monotonic nanoseconds when cycle frequency is unavailable
- monotonic clamp prevents backwards time progression

Architecture hook:
- `arch_timer_clocksource_hz(const boot_info_t *)` added to `arch_timer` API
