# 300 User-Mode Substrate Scope and Non-Goals

## Objective
Define exactly what Monolith provides for user-mode bring-up as reusable mechanism, not OS policy.

## Multi-Architecture Closure Rule
This spec is part of a tri-arch program and is only globally complete when corresponding architecture-facing specs also complete for `x86_64`, `arm64`, and `riscv64`.

## In Scope
- Arch-neutral user-mode entry interface.
- Kernel return-stack wiring interface.
- Generic user-task bootstrap primitives.
- Shared user-memory access safety helpers.
- Scheduler pluggability contract.
- Trap/syscall register mapping helpers.

## Out of Scope
- Shells, commands, CLI UX.
- Process tree, pid model, signal model.
- VFS/filesystem policy.
- Userland packaging/runtime policy.

## Design Inputs from Existing OS Bring-Up Work
- An OS-level `launch_init_task` function showed the minimum viable launch sequence.
- A scan-based `find_stack_page` helper exposed why heuristic stack search should move to substrate allocator APIs.
- `x86_64_user_mode_set_kernel_stack` and `x86_64_user_mode_enter` are reusable mechanisms.
- Syscall pointer checks in `kernel/syscall.c` proved shared uaccess API value.

## Reference Sequence (Smoke/Test and Documentation)
Use this as a smoke-test helper and documentation example only.
Do not require Monolith to expose `launch_first_user_task` as core API; production OS layers should launch tasks via their own policy code.

```c
status_t launch_first_user_task(const boot_info_t *bi, user_entry_fn entry) {
  u64 page_size = mm_page_size();
  u64 stack_page = user_stack_alloc_page(page_size);
  u64 user_sp = stack_page + page_size - 16ULL;

  status_t st = mm_protect(USER_BASE, USER_SIZE, MMU_PROT_READ | MMU_PROT_WRITE | MMU_PROT_EXEC | MMU_PROT_USER);
  if (st != STATUS_OK) return st;
  st = mm_sync_tlb(USER_BASE, USER_SIZE);
  if (st != STATUS_OK) return st;

  st = syscall_set_boot_info(bi);
  if (st != STATUS_OK) return st;
  st = syscall_set_user_window(USER_BASE, USER_SIZE);
  if (st != STATUS_OK) return st;

  st = arch_user_mode_set_kernel_stack(kernel_stack_top());
  if (st != STATUS_OK) return st;
  arch_user_mode_enter(entry, (void *)0, user_sp);
}
```

## Documentation Note
This spec should produce a companion article:
- "From booted kernel to first user task: mechanism vs OS policy".

## Acceptance Criteria
1. Scope is documented with strict mechanism vs policy separation.
2. Substrate APIs and ownership boundaries are listed.
3. Non-goals are explicit to prevent feature creep.
