# 350 Generic User Task Bootstrap Helper

## Objective
Provide substrate helpers for first user task creation without OS-specific heuristics.

## Multi-Architecture Closure Rule
This spec is complete only when helper contracts work across `x86_64`, `arm64`, and `riscv64` with architecture-specific backends.

## In Scope
- User stack allocation helper API.
- User window setup helper API.
- Launch context preparation helper API.
- Optional convenience flow for one-task bootstrap.

## Out of Scope
- Process tree creation policy.
- Userspace loader format policy.
- Shell command behavior.

## Implementation Tasks
1. Introduce allocator-backed user-stack API.
2. Replace ad-hoc stack-search pattern with reusable primitive.
3. Define preconditions/postconditions for launch helpers.
4. Document minimal launch sequence for OS personalities.

## Design Inputs from Existing OS Bring-Up Work
- A scan-based `find_stack_page` helper should be replaced with allocator contract.
- A hand-written `launch_init_task` sequence should map to reusable helper calls.
- `mm_protect` and `mm_sync_tlb` ordering should be codified.

## Required Helper API (Copy Into Monolith)

```c
typedef struct {
  BOOT_U64 user_base;
  BOOT_U64 user_size;
  BOOT_U64 user_ip;
  BOOT_U64 user_sp;
  void *kernel_stack_top;
} user_task_bootstrap_t;

status_t user_stack_alloc(BOOT_U64 size, BOOT_U64 *out_base);
status_t user_window_map(BOOT_U64 base, BOOT_U64 size, BOOT_U64 prot);
status_t user_task_bootstrap_prepare(const boot_info_t *bi, user_task_bootstrap_t *out_ctx);
```

Heuristic to replace:

```c
/* Replace scan-based stack page selection with allocator-backed API. */
```

## Acceptance Criteria
1. OS layer can launch a first user task without manual page scanning.
2. Helpers return explicit errors on allocation/mapping/setup failure.
3. Documentation includes one canonical launch sequence.
