# Per-CPU Runtime

API: `kernel/include/percpu.h`  
Implementation: `kernel/percpu.c`

## Purpose

The per-CPU layer provides a stable, lockless way to access CPU-local runtime state before scheduler/SMP work exists.

This is required for:
- interrupt nesting accounting
- local timer tick accounting
- future scheduler and SMP correctness

## Data Structure

`percpu_t` fields:
- `cpu_id`: architecture CPU identifier for this slot.
- `online`: `1` when slot is active.
- `irq_nesting`: incremented during interrupt dispatch, decremented on exit.
- `preempt_disable_depth`: reserved for scheduler/preemption policy.
- `local_tick_count`: per-CPU timer interrupts observed.
- `current_task`: reserved pointer for scheduler integration.
- `arch_local`: reserved architecture-private pointer.

## Initialization Contract

Prototype: `status_t percpu_init_boot_cpu(const boot_info_t *boot_info)`

Rules:
- no dynamic allocator usage
- safe during early boot
- must be called after `arch_cpu_early_init` and before interrupt/timer bring-up

Behavior:
1. clears static per-CPU table (`PERCPU_MAX_CPUS`).
2. chooses boot CPU ID (`boot_info.boot_cpu_id` when valid, else `arch_cpu_id()`).
3. initializes slot 0 and marks it online.
4. writes CPU-local base register to slot 0 address via `arch_cpu_set_local_base`.

## Accessors

- `percpu_t *percpu_current(void)`
  - lockless read of current CPU-local pointer via architecture base register.
  - returns `NULL` if not initialized.
- `percpu_t *percpu_by_id(u64 cpu_id)`
  - linear search over online slots.
  - intended for early/simple use (SMP scaling can evolve later).

## Architecture Hook Mapping

Defined in `kernel/include/arch_cpu.h`:
- `status_t arch_cpu_set_local_base(u64 base)`
- `u64 arch_cpu_get_local_base(void)`

Current backends:
- x86_64: GS base MSR (`IA32_GS_BASE`)
- arm64: `TPIDR_EL1`
- riscv64: `tp` register

## Where It Is Used Today

- `kernel/interrupts.c`: updates `percpu_current()->irq_nesting`
- `kernel/timebase.c`: updates `percpu_current()->local_tick_count` in timer IRQ path

These are the first required correctness hooks for future SMP and scheduler work.
