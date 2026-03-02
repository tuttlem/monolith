# Monolith Developer Manual

This manual explains the current platform bring-up base for:
- `x86_64`
- `arm64`
- `riscv64`

Use this as the reference for boot handoff, memory initialization, MMU mapping, per-CPU runtime, allocators, interrupts, timers, and diagnostics.

## Read This First

1. [Core Interfaces (spec 010)](core/core-interfaces.md)
2. [Status, Assert, Panic (spec 020)](core/status-system.md)
3. [Config and Feature Flags (spec 030)](core/config-flags.md)
4. [CPU Layer (`arch_cpu`, spec 040)](core/cpu-layer.md)
5. [Exception and Interrupt Framework (spec 050)](core/exceptions.md)
6. [Interrupt Controller Layer (spec 060)](core/irq-controller.md)
7. [Time System (spec 070)](core/time-system.md)
8. [Memory Stack + MMU Mapping API (spec 080)](core/memory.md)
9. [Per-CPU Runtime (spec 090)](core/percpu.md)
10. [SMP Bootstrap Skeleton (spec 100)](core/smp-bootstrap.md)
11. [Unified Device Discovery (spec 110)](core/device-discovery.md)
12. [Core Boot Sequence](core/boot-sequence.md)
13. [Boot ABI (`boot_info_t`)](core/boot-info-abi.md)
14. [API Cheatsheet](core/api-cheatsheet.md)
15. [Spec Coverage Matrix (010-110)](core/spec-coverage-010-110.md)

## Architecture-Specific Manuals

- [x86_64 Manual](x86_64/README.md)
- [arm64 Manual](arm64/README.md)
- [riscv64 Manual](riscv64/README.md)

## Practical Usage Model

When starting a new OS on top of this base, your normal first integrations are:
1. Keep boot protocol and `boot_info_t` stable.
2. Bring up CPU-local runtime (`arch_cpu_early_init`, `percpu_init_boot_cpu`).
3. Bring up memory (`arch_mm_early_init`, `page_alloc_init`, `kmalloc_init`).
4. Bring up interrupts and timer (`interrupts_init`, `timer_init`).
5. Add your platform-independent subsystems on top of `kmalloc`, `status_t`, per-CPU accessors, and interrupt/timer callbacks.

The current code path that orchestrates this is `kmain` in `kernel/kmain.c`.
