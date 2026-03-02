# riscv64 Manual

## Scope

This chapter explains current riscv64 behavior for:
- boot handoff construction from hart + DTB entry
- early paging takeover via `satp`
- interrupt backend (`stvec` + trap bridge)
- timer status and current defer behavior

## Entry Files

- Boot: `arch/riscv64/boot/main.c`, `arch/riscv64/start.S`
- Memory init: `arch/riscv64/mm/memory_init.c`
- Early paging: `arch/riscv64/mm/early_paging.c`
- Interrupt backend: `arch/riscv64/irq/entry.S`, `arch/riscv64/irq/interrupts.c`
- Timer backend: `arch/riscv64/timer/timer.c`

## Architecture Chapters

- [Boot](boot.md)
- [Memory](memory.md)
- [Interrupts and Timer](interrupts-timer.md)
