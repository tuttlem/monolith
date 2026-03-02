# Exception and Interrupt Framework (Spec 050)

Core API: `kernel/include/interrupts.h`  
Panic API: `kernel/include/panic.h`  
Core implementation: `kernel/interrupts.c`, `kernel/panic.c`

## Dispatch Model

1. architecture trap/IRQ entry builds `interrupt_frame_t`
2. architecture bridge forwards into `interrupts_dispatch`
3. generic dispatcher routes registered vectors
4. unhandled faults route to unified panic path
5. unhandled IRQ vectors are log-once (if enabled)

## Exception Classification

`exception_info_t` carries normalized metadata:
- class id (`EXCEPTION_CLASS_*`)
- architecture id
- vector
- decoded error code and raw syndrome
- fault address, instruction pointer, stack pointer, flags
- textual reason

Current unhandled exception policy:
- panic with formatted metadata output
- interrupts disabled
- stop via `arch_panic_stop()`

## Assert and Panic Policy (Spec 020)

`ASSERT(expr)` comes from `kernel/include/assert.h` and is controlled by:
- `MONOLITH_ASSERT_ENABLE`
- `MONOLITH_ASSERT_PANIC`

Panic entrypoints:
- `panic(const char *reason)`
- `panicf(const char *fmt, ...)`
- `panic_from_exception(const exception_info_t *info)`

Panic output includes:
- reason string
- architecture id
- CPU id
- exception register/fault metadata when available
- stacktrace placeholder (`<stub>`)
