# Scheduler Scaffolding

This layer provides a minimal task/run-queue substrate without imposing OS scheduling policy.

## Purpose

- Give downstream OS code a stable scheduler API surface early.
- Keep architecture switch mechanics behind `arch_context_switch`.
- Support single-task bring-up while leaving room for richer schedulers.

## Public API

Header:
- `kernel/include/scheduler.h`

Types:
- `task_state_t`
- `task_t`

Functions:
- `status_t sched_init(void)`
- `task_t *sched_current(void)`
- `status_t sched_add(task_t *task)`
- `void sched_tick(void)`
- `status_t arch_context_switch(task_t *from, task_t *to)`

## Remarks

- Current implementation starts with one internal idle task.
- `sched_tick` rotates runnable tasks and invokes `arch_context_switch` when task ownership changes.
- Architecture backends currently route through `cpu_context_switch` for deterministic bring-up behavior.
