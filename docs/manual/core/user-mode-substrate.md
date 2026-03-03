# User-Mode Substrate Scope

This page defines what Monolith provides for user-mode bring-up as reusable mechanism, and what is intentionally left to OS personalities.

## Design Boundary

Monolith user-mode support is a bootstrap substrate:
- it provides architecture-neutral entry/dispatch contracts
- it provides shared safety helpers for user-pointer access
- it provides task-bootstrap helpers for first-user-task wiring

Monolith does not define process policy:
- no process tree model
- no pid policy
- no signal model
- no filesystem policy
- no shell/userland packaging model

## What Monolith Owns

The substrate owns mechanism APIs that OS personalities can call:
- `arch_user_mode_set_kernel_stack`
- `arch_user_mode_enter`
- optional architecture frame-prep helpers
- checked user-memory copy helpers (`uaccess`)
- user task bootstrap helpers (`user_stack_alloc`, `user_window_map`, `user_task_bootstrap_prepare`)

These APIs are intentionally neutral and versionable so different OS designs can reuse the same hardware-facing base.

## What OS Personalities Own

OS personality code owns policy:
- when to create tasks/processes
- how to schedule user entities
- how to define syscall namespaces above substrate-reserved ranges
- how to represent executable images and loader behavior

Monolith provides execution hooks; personality modules decide semantics.

## Canonical Sequence (Mechanism Only)

The substrate sequence is:
1. allocate user stack pages from page allocator-backed helper
2. map/protect user virtual window through MMU API
3. configure syscall/uaccess user window constraints
4. wire kernel return stack for trap/syscall return path
5. enter user mode through architecture backend

This sequence is a reference bring-up mechanism. It is not process policy.

## Practical Rule

If a decision changes OS semantics (signals, file descriptors, process hierarchy, permissions model), it belongs outside Monolith.

If a decision is required to safely transition privilege level or move bytes across user/kernel boundary, it belongs inside Monolith.
