# 020 Status, Assertions, and Panic Policy

## Goal

Standardize all error and fatal paths early to avoid broad refactors later.

## Existing Base

`status_t` exists and is already used by memory/irq/timer init code.

## Required Extensions

1. Panic subsystem:
- `panic(const char *reason)`
- `panicf(const char *fmt, ...)`
- `panic_from_exception(const exception_info_t *info)`

2. Assertion model:
- debug assert (`ASSERT(expr)`)
- release assert policy toggle (panic vs compile-out)

3. Error taxonomy guidance:
- retain current `status_t` values
- reserve ranges for subsystem-local statuses if needed

## Panic Output Requirements

Panic path must print:
- reason string
- architecture id
- CPU id (if available)
- exception/interrupt metadata when present
- fault address and instruction pointer where available

Stack trace can be a stub in this phase, but API and output slot must exist.

## Reentrancy/Fault-Safety Rules

- panic path must tolerate interrupts being disabled.
- panic path must be safe if allocator is unavailable.
- panic path must not recurse infinitely on secondary faults.

## Architecture Backend Hooks

- `arch_panic_stop()` hard-stop primitive
- optional `arch_panic_dump_regs(...)`

## Acceptance Criteria

- all fatal exception paths route through unified panic backend
- all subsystem init failures return `status_t` and avoid hard-stop unless unrecoverable
- panic output format consistent across architectures
