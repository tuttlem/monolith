# 360 User Memory Access API Promotion

## Objective
Standardize checked user-memory copy as a first-class Monolith API.

## Multi-Architecture Closure Rule
This spec is complete only when syscall/uaccess paths on `x86_64`, `arm64`, and `riscv64` use shared checked-copy APIs.

## In Scope
- `copy_from_user_checked` and `copy_to_user_checked` style helpers.
- User-range validation against configured user window.
- Common error semantics for invalid pointer and range overflow.

## Out of Scope
- Copy optimization/micro-architecture tuning.
- Userland memory allocator policy.
- Scheduler/process policy.

## Implementation Tasks
1. Move shared logic into core module.
2. Define public API contract in core include path.
3. Refactor syscall consumers to use shared helpers.
4. Add negative tests for invalid user pointers and bounds.

## Required API (Copy Into Monolith)

```c
status_t uaccess_set_user_window(BOOT_U64 base, BOOT_U64 size);
status_t copy_from_user_checked(void *dst, BOOT_U64 user_src, BOOT_U64 len);
status_t copy_to_user_checked(BOOT_U64 user_dst, const void *src, BOOT_U64 len);
```

## Validation Pattern

```c
static int user_range_ok(BOOT_U64 p, BOOT_U64 n) {
  if (n == 0) return 1;
  if (p < g_user_base) return 0;
  if (p + n < p) return 0; /* overflow */
  if (p + n > g_user_base + g_user_size) return 0;
  return 1;
}
```

## Design Inputs from Existing OS Bring-Up Work
- Existing checked-copy pattern in syscall path.
- User-window validation sufficient for phase-1 safety.

## Acceptance Criteria
1. No direct user pointer dereference remains in syscall handlers.
2. Invalid pointers return deterministic status without kernel panic.
3. API is reusable by multiple OS personalities.
