# Generic User Task Bootstrap

This page documents the neutral helper APIs used to prepare the first user task without OS-specific stack-scanning heuristics.

## Purpose

The helper set provides a reusable launch mechanism:
- allocate user stack memory from page allocator-backed APIs
- apply user-mode MMU protections on a launch window
- return a normalized launch context struct

The helper set does not define process policy, loader policy, or task hierarchy.

## APIs

- `status_t user_stack_alloc(u64 size, u64 *out_base)`
- `status_t user_window_map(u64 base, u64 size, u64 prot)`
- `status_t user_task_bootstrap_prepare(const boot_info_t *boot_info, user_task_bootstrap_t *out_ctx)`

## Canonical Usage

```c
user_task_bootstrap_t ctx;
status_t st = user_task_bootstrap_prepare(boot_info, &ctx);
if (st != STATUS_OK) {
  return st;
}

st = arch_user_mode_set_kernel_stack(ctx.kernel_stack_top);
if (st != STATUS_OK) {
  return st;
}

arch_user_mode_enter(entry_fn, (void *)0, ctx.user_sp);
```

## Current Phase Notes

- The helper currently builds a minimal single-page launch window and stack.
- OS personalities can replace address layout policy while keeping the helper call contract.
