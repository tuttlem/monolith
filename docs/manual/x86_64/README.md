# x86_64 Manual

## Scope

This chapter explains current x86_64 behavior for:
- UEFI handoff construction
- early paging takeover
- interrupt backend (IDT + exception/IRQ stubs)
- timer backend (PIC + PIT)

## Entry Files

- Boot: `arch/x86_64/boot/efi_main.c`
- Memory init: `arch/x86_64/mm/memory_init.c`
- Early paging: `arch/x86_64/mm/early_paging.c`
- Interrupt backend: `arch/x86_64/irq/interrupts.c`
- Timer backend: `arch/x86_64/timer/timer.c`

## Architecture Chapters

- [Boot](boot.md)
- [Memory](memory.md)
- [Interrupts and Timer](interrupts-timer.md)
- [Architecture API Reference](api-reference.md)
