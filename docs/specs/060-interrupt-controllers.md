# 060 Interrupt Controller Drivers

## Goal

Introduce real controller-driver layer under generic IRQ framework.

## Driver Model

Define interrupt-controller ops:
- `init`
- `enable_irq(irq)`
- `disable_irq(irq)`
- `ack_irq(irq)`
- `eoi_irq(irq)` (if distinct)
- `set_affinity` (optional stub for future SMP)

Provide a generic IRQ domain mapping:
- hardware IRQ number -> logical vector

## Architecture Plans

### x86_64
Phase out legacy PIC as default path:
1. bootstrap with LAPIC timer/IPI support path
2. IOAPIC routing for external interrupts
3. keep PIC fallback only for bring-up mode

### arm64
Formalize GIC driver split:
- distributor
- redistributor/cpu-interface (v2/v3-aware layering)
- interrupt id translation and affinity hooks

### riscv64
Introduce platform abstraction for interrupt source:
- PLIC/APLIC style external interrupts (platform dependent)
- per-hart local interrupts path
- keep backend pluggable to machine model

## Generic API Boundary

Generic `interrupts_*` must not directly access controller MMIO or CSR details.

## Acceptance Criteria

- timer IRQ path uses controller-driver abstractions
- controller init ordering documented relative to `arch_cpu` and discovery data
- architecture backends expose explicit IRQ capability/limits
