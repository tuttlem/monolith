# Core Interfaces (Spec 010)

This project freezes minimal architecture HAL interfaces so generic kernel code avoids architecture-specific headers and inline assembly.

## Frozen Interface Families

- `arch_cpu`: `kernel/include/arch_cpu.h`
- `arch_irq`: `kernel/include/arch_irq.h`
- `arch_timer`: `kernel/include/arch_timer.h`
- `arch_mm`: `kernel/include/arch_mm.h`

Related generic services:
- `status`: `kernel/include/status.h`
- panic/assert: `kernel/include/panic.h`, `kernel/include/assert.h`
- interrupts: `kernel/include/interrupts.h`
- time: `kernel/include/timebase.h`

## Versioning Rule

Each exported architecture interface header defines:
- `*_API_VERSION_MAJOR`
- `*_API_VERSION_MINOR`

Compatibility policy:
- major bump for breaking API changes
- minor bump for additive extensions

## Generic Initialization Contract

Current expected order in `kmain`:
1. `arch_cpu_early_init`
2. `percpu_init_boot_cpu`
3. `arch_mm_early_init`
4. allocators (`page_alloc_init`, `kmalloc_init`)
5. device model registration/probe (`driver_registry_reset`, `device_model_register_builtin_drivers`, `driver_probe_all`)
6. IRQ/timer come through device-model classes (`irqc`, `timer`)

## Design Constraints for Interface Changes

- function contracts must use `status_t` for fallible operations
- do not expose mutable backend globals in public headers
- generic modules must compile without architecture-specific includes
