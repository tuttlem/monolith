# arm64 Manual

## Scope

This chapter explains current arm64 behavior for:
- UEFI handoff construction
- early paging takeover (TTBR0/MAIR/TLB maintenance)
- interrupt backend (VBAR + GICv2 CPU interface)
- timer backend (generic virtual timer)

## Entry Files

- Boot: `arch/arm64/boot/efi_main.c`
- Memory init: `arch/arm64/mm/memory_init.c`
- Early paging: `arch/arm64/mm/early_paging.c`
- Interrupt backend: `arch/arm64/irq/interrupts.c`, `arch/arm64/irq/entry.S`
- Timer backend: `arch/arm64/timer/timer.c`

## Architecture Chapters

- [Boot](boot.md)
- [Memory](memory.md)
- [Interrupts and Timer](interrupts-timer.md)
