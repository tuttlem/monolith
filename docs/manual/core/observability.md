# Observability Substrate

This layer provides an always-available trace ring for early bring-up diagnostics.

## Purpose

- Capture key events across boot and runtime without external tooling.
- Keep trace format architecture-neutral.
- Surface recent events during panic for post-mortem analysis.

## Public API

Header:
- `kernel/include/trace.h`

Functions:
- `status_t trace_init(const boot_info_t *boot_info)`
- `void trace_emit(trace_class_t cls, BOOT_U64 a0, BOOT_U64 a1, BOOT_U64 a2)`
- `status_t trace_dump(trace_sink_t sink)`
- `void trace_sink_kprintf(const trace_record_t *record)`

## Event Classes

- `TRACE_CLASS_IRQ_ENTRY`
- `TRACE_CLASS_IRQ_EXIT`
- `TRACE_CLASS_SYSCALL`
- `TRACE_CLASS_TIMER`
- `TRACE_CLASS_PANIC`
- `TRACE_CLASS_DEVICE`

## Current Integration

- IRQ dispatch emits entry and exit events.
- Syscall dispatch emits operation and status.
- Timer path emits periodic timebase progress events.
- Panic path emits panic event and dumps recent trace records.
- Device probe path emits probe status summary in `kmain`.
