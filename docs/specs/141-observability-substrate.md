# 141 Observability Substrate

## Goal

Add a lightweight, always-available kernel observability layer for bring-up diagnostics and post-mortem analysis.

## In Scope

- Lock-aware event ring buffer with timestamped records.
- Standard event classes:
  - irq entry/exit
  - syscall op + status
  - timer ticks/calibration
  - fault/panic summary
  - device probe results.
- Panic dump integration to serial/log sink.

## Out of Scope

- Full tracing UI or userspace tooling.

## Public Interfaces

- Header: `kernel/include/trace.h`
- APIs:
  - `status_t trace_init(const boot_info_t *boot_info)`
  - `void trace_emit(trace_class_t cls, BOOT_U64 a0, BOOT_U64 a1, BOOT_U64 a2)`
  - `status_t trace_dump(trace_sink_t sink)`

## Tests

- ring wrap and overflow behavior tests.
- panic path includes last N events.

## Acceptance Criteria

1. Trace capture works on all three architectures from early boot onward.
2. No boot regression when tracing disabled via config flag.
