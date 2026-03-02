# Status System (`status_t`)

Definition: `kernel/include/status.h`
Implementation of names: `kernel/status.c`

## Type

- `typedef int status_t;`

## Constants

- `STATUS_OK = 0`
- `STATUS_DEFERRED = 1`
- Negative failures:
  - `STATUS_INVALID_ARG`
  - `STATUS_NOT_FOUND`
  - `STATUS_NO_MEMORY`
  - `STATUS_NOT_SUPPORTED`
  - `STATUS_BUSY`
  - `STATUS_FAULT`
  - `STATUS_INTERNAL`
  - `STATUS_TRY_AGAIN`

## Semantics

- `0`: success, operation completed.
- `>0`: non-fatal deferral; subsystem intentionally not active yet.
- `<0`: failure.

Use helper:
- `status_is_ok(st)` returns true only for `STATUS_OK`.

For logs:
- `status_str(st)` returns static string names.

## Where `status_t` Is Central Today

- `arch_memory_init`
- `page_alloc_init`
- `kmalloc_init`
- `interrupts_init`
- `timer_init`
- `arch_interrupts_init`
- `arch_timer_init`
- `arch_cpu_early_init`
- `percpu_init_boot_cpu`
- MMU mapping API (`mm_map`, `mm_unmap`, `mm_protect`, `mm_translate`)

## Coding Guidance

1. Return `STATUS_INVALID_ARG` for bad pointers/inputs.
2. Return `STATUS_DEFERRED` when the subsystem is intentionally not ready yet (not an error).
3. Use `STATUS_INTERNAL` for logic failures that should not happen in normal operation.
4. Use `STATUS_NOT_SUPPORTED` for architecture/platform mismatch.

## Panic and Assert Relation (Spec 020)

`status_t` is for recoverable/deferrable control flow.  
`panic*` is for unrecoverable failures.

Related APIs:
- `panic`, `panicf`, `panic_from_exception` (`kernel/include/panic.h`)
- `ASSERT(expr)` (`kernel/include/assert.h`)

Policy:
- return `status_t` when caller can decide behavior
- use panic for invariant or fatal trap paths only
