# 370 Scheduler Interface Contract and Default Backend

## Objective
Make scheduler replacement smooth by defining a formal scheduler operations contract.

## Multi-Architecture Closure Rule
This spec is complete only when the scheduler interface compiles and integrates on `x86_64`, `arm64`, and `riscv64`.

## In Scope
- Scheduler ops interface (init, select, set foreground, yield/exit hooks).
- Default serial backend preserving current behavior.
- Registration or selection mechanism for alternate backends.

## Out of Scope
- Full parallel scheduler implementation.
- SMP policy expansion.
- Process policy and fairness model.

## Implementation Tasks
1. Define scheduler interface and ownership model.
2. Wrap current default scheduler behind the interface.
3. Preserve `sched_set_foreground` behavior through interface.
4. Document OS personality integration steps for backend swap.

## Required Scheduler Interface (Copy Into Monolith)

```c
typedef struct task task_t;

typedef struct {
  status_t (*init)(void);
  status_t (*set_foreground)(task_t *task);
  status_t (*enqueue)(task_t *task);
  task_t *(*pick_next)(void);
  void (*on_yield)(task_t *task);
  void (*on_exit)(task_t *task, u64 code);
} scheduler_ops_t;

status_t sched_register_backend(const scheduler_ops_t *ops);
```

Compatibility target:
- existing `sched_set_foreground(...)` call sites should continue to work unchanged.

## Design Inputs from Existing OS Bring-Up Work
- `sched_set_foreground` is a proven seam.
- Single-task serial policy should remain default backend.

## Acceptance Criteria
1. Existing behavior is unchanged under default backend.
2. Alternate backend can be swapped without rewriting call sites.
3. Interface lifecycle and error semantics are documented.
