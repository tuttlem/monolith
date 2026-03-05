# 090 CPU-Local Data and Per-CPU Runtime

## Goal

Define a minimal per-CPU data framework required for SMP-safe evolution.

## Per-CPU Structure (Minimum)

```c
typedef struct {
  u64 cpu_id;
  u64 online;
  u64 irq_nesting;
  u64 preempt_disable_depth;
  u64 local_tick_count;
  void *current_task;
  void *arch_local;
} percpu_t;
```

## Required APIs

- `status_t percpu_init_boot_cpu(const boot_info_t *boot_info)`
- `percpu_t *percpu_current(void)`
- `percpu_t *percpu_by_id(u64 cpu_id)`

Architecture hook required:
- set/get per-CPU base register mechanism.

## Architecture Mapping

- x86_64: GS-base/CPU-local base strategy
- arm64: TPIDR_EL1-based strategy
- riscv64: `tp` register or equivalent supervisor-local base

## Rules

- no dynamic allocator required for boot CPU setup
- lockless current-CPU lookup path
- valid before full scheduler bring-up

## Acceptance Criteria

- interrupt and timer paths can read/update per-CPU tick/nesting counters
- per-CPU accessor used instead of global singleton counters where appropriate
