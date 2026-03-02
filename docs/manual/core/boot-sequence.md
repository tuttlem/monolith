# Core Boot Sequence

## Entry Contract

Kernel entry is:
- `void kmain(const boot_info_t *boot_info)`
- defined in `kernel/kmain.c`

Bootloader/firmware glue is architecture-specific and must construct `boot_info_t` before calling `kmain`.

## Current Startup Order (Inside `kmain`)

Initialization order is fixed and intentional:
1. `arch_cpu_early_init(boot_info)`
2. `percpu_init_boot_cpu(boot_info)`
3. `smp_init(boot_info)`
4. `arch_mm_early_init(boot_info_mut)` (wrapper over `arch_memory_init`)
5. `page_alloc_init(boot_info_mut)`
6. `kmalloc_init(boot_info_mut)`
7. `interrupts_init(boot_info)`
8. `timer_init(boot_info)`

Then optional self-tests/diagnostics run (macro-controlled), then the kernel idles in `arch_halt()` loop.

## Why This Order

- `arch_cpu_early_init` provides stable CPU identity and architecture CPU state.
- `percpu_init_boot_cpu` installs a lockless current-CPU pointer before IRQ/timer activity.
- `smp_init` defines possible/online CPU topology and runs architecture SMP bootstrap when enabled.
- `arch_mm_early_init` may change page table root and records final VM state in `boot_info`.
- `page_alloc_init` depends on usable memory regions in `boot_info.memory_regions`.
- `kmalloc_init` depends on page allocator availability.
- `interrupts_init` sets the CPU interrupt backend and handler table; dispatch updates per-CPU IRQ nesting.
- `timer_init` registers a timer IRQ handler and enables interrupts when successful; timer IRQ updates per-CPU local tick count.

## Deferred vs Hard Failure

The project uses `status_t` semantics:
- hard failures: negative status values (`STATUS_* < 0`)
- deferred/optional bring-up: `STATUS_DEFERRED` (`> 0`)
- success: `STATUS_OK` (`0`)

`kmain` logs hard failures and continues to allow partial bring-up and diagnostics.

## Primary Boot Entry Files

- x86_64 UEFI: `arch/x86_64/boot/efi_main.c`
- arm64 UEFI: `arch/arm64/boot/efi_main.c`
- riscv64 non-UEFI path: `arch/riscv64/boot/main.c`
