# 310 Architecture-Neutral User-Mode API Contract

## Objective
Introduce a stable core interface for entering user mode and wiring kernel return context.

## Multi-Architecture Closure Rule
This spec is complete only when the API contract is consumed by all backend specs (`x86_64`, `arm64`, `riscv64`) and all targets compile.

## In Scope
- `arch_user_mode_set_kernel_stack(void *top)` contract.
- `arch_user_mode_enter(entry_fn, arg, user_sp)` contract.
- Optional frame preparation contract when required by architecture.

## Out of Scope
- Shell/init policy.
- Process scheduler policy.
- Architecture-specific implementation details.

## Implementation Tasks
1. Add core header with API contract and invariants.
2. Define argument/return semantics and lifetime rules.
3. Define deterministic error returns for unsupported/invalid paths.
4. Provide compile-safe stubs for backends not yet implemented.

## Required API (Copy Into Monolith)

```c
typedef void (*arch_user_entry_t)(void *arg);

status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top);
__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, BOOT_U64 user_sp);
```

Optional frame-oriented API if your backend prefers explicit register frames:

```c
typedef struct {
  BOOT_U64 user_ip;
  BOOT_U64 user_sp;
  BOOT_U64 arg0;
  BOOT_U64 flags;
} arch_user_frame_t;

status_t arch_user_mode_prepare_frame(arch_user_frame_t *frame);
```

## Stub Behavior (For Incomplete Backends)

```c
status_t arch_user_mode_set_kernel_stack(void *top) {
  (void)top;
  return STATUS_NOT_SUPPORTED;
}
```

## Acceptance Criteria
1. API compiles on all supported architectures.
2. Contract clearly separates core mechanism from OS policy.
3. Unsupported paths return deterministic `STATUS_NOT_SUPPORTED`.
