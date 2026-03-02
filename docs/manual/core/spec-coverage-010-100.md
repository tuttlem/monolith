# Spec Coverage Matrix (010-100)

This matrix links each implemented foundation spec to the manual sections that define its operational contract.

## Coverage

- `010-core-interfaces`
  - `core/core-interfaces.md`
  - `core/boot-sequence.md`
  - `core/api-cheatsheet.md`

- `020-status-panic-policy`
  - `core/status-system.md`
  - `core/exceptions.md`

- `030-config-feature-flags`
  - `core/config-flags.md`
  - `core/boot-sequence.md`

- `040-arch-cpu`
  - `core/cpu-layer.md`
  - `core/percpu.md`
  - `core/api-cheatsheet.md`

- `050-exception-interrupt-framework`
  - `core/exceptions.md`
  - `core/interrupts-timers.md`

- `060-interrupt-controllers`
  - `core/irq-controller.md`
  - `core/interrupts-timers.md`

- `070-time-system`
  - `core/time-system.md`
  - `core/interrupts-timers.md`
  - `core/api-cheatsheet.md`

- `080-mmu-mapping-api`
  - `core/memory.md`
  - `x86_64/memory.md`
  - `arm64/memory.md`
  - `riscv64/memory.md`

- `090-per-cpu`
  - `core/percpu.md`
  - `core/boot-sequence.md`
  - `core/interrupts-timers.md`

- `100-smp-bootstrap`
  - `core/smp-bootstrap.md`
  - `core/boot-sequence.md`
  - `core/api-cheatsheet.md`

## Reader Usage

Use this order for a complete foundation pass:
1. `core/core-interfaces.md`
2. `core/status-system.md`
3. `core/config-flags.md`
4. `core/cpu-layer.md`
5. `core/exceptions.md`
6. `core/irq-controller.md`
7. `core/time-system.md`
8. `core/memory.md`
9. `core/percpu.md`
10. `core/smp-bootstrap.md`
11. architecture memory and interrupt chapters
