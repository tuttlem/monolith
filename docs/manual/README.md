# Monolith Developer Manual

This manual explains the current platform bring-up base for:
- `x86_64`
- `arm64`
- `riscv64`

Use this as the reference for boot handoff, memory initialization, allocators, interrupts, timers, and diagnostics.

## Read This First

1. [Core Boot Sequence](core/boot-sequence.md)
2. [Boot ABI (`boot_info_t`)](core/boot-info-abi.md)
3. [Status System (`status_t`)](core/status-system.md)
4. [Memory Stack (arch memory init -> page alloc -> kmalloc)](core/memory.md)
5. [Interrupts and Timers](core/interrupts-timers.md)
6. [API Cheatsheet](core/api-cheatsheet.md)

## Architecture-Specific Manuals

- [x86_64 Manual](x86_64/README.md)
- [arm64 Manual](arm64/README.md)
- [riscv64 Manual](riscv64/README.md)

## Practical Usage Model

When starting a new OS on top of this base, your normal first integrations are:
1. Keep boot protocol and `boot_info_t` stable.
2. Bring up memory (`arch_memory_init`, `page_alloc_init`, `kmalloc_init`).
3. Bring up interrupts and timer (`interrupts_init`, `timer_init`).
4. Add your platform-independent subsystems on top of `kmalloc`, `status_t`, and interrupt/timer callbacks.

The current code path that orchestrates this is `kmain` in `kernel/kmain.c`.
