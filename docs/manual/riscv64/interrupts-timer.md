# riscv64 Interrupts and Timer

## Interrupt Backend

Files:
- `arch/riscv64/irq/entry.S`
- `arch/riscv64/irq/interrupts.c`

### Setup

- `arch_interrupts_init` writes `stvec` to `riscv64_trap_entry`.

### Trap flow

`riscv64_trap_c(scause, sepc, stval, sstatus, sp_at_trap)` builds `interrupt_frame_t`:
- interrupt causes map to vectors `32 + cause_code`
- synchronous exceptions map to vector `cause_code`
- fields include `scause`, `stval`, `sepc`, `sstatus`, and trap-time stack pointer

Then it calls `interrupts_dispatch`.

### Self-test exception trigger

`arch_exception_selftest_trigger()` executes `ebreak`.

## Timer Backend (Current Status)

File: `arch/riscv64/timer/timer.c`

Current behavior:
- validates args and arch id
- returns `STATUS_DEFERRED`
- provides no active timer vector yet

Implication:
- `timer_init` will report deferred on riscv64 until a real timer source (e.g., SBI timer or platform CLINT/ACLINT path) is wired.
