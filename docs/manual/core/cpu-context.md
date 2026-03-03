# CPU Feature and Context Facilities

This layer exposes architecture-neutral capability queries and context object helpers.

## Purpose

- Let generic subsystems query CPU feature presence without architecture `#ifdef` usage.
- Provide a stable context object contract for future scheduler/runtime work.
- Keep task/process policy out of the substrate.

## Public API

Headers:
- `kernel/include/cpu_caps.h`
- `kernel/include/cpu_context.h`

APIs:
- `status_t cpu_caps_query(cpu_caps_t *out_caps)`
- `status_t cpu_context_init(cpu_context_t *ctx, void (*entry)(void *), void *arg, void *stack_top)`
- `status_t cpu_context_switch(cpu_context_t *from, cpu_context_t *to)`

## Notes

- Current context switch helper is a deterministic state swap primitive.
- Architecture-specific save/restore backends can replace internals later without changing signatures.
