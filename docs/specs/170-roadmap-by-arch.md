# 170 Architecture Roadmap and Parity Matrix

## Goal

Translate the specs into an execution matrix so progress is measurable per architecture.

## Legend

- `P0`: must-have for usable base
- `P1`: strongly recommended in base
- `P2`: can be deferred after base milestone

## Capability Matrix

| Capability | x86_64 | arm64 | riscv64 | Priority |
|---|---|---|---|---|
| `arch_cpu` primitives | required | required | required | P0 |
| exception decode + panic metadata | required | required | required | P0 |
| interrupt controller layer | LAPIC/IOAPIC + fallback | GIC driver layer | PLIC/APLIC/SBI-local abstraction | P0 |
| clocksource + clockevent | TSC + APIC/PIT fallback | generic counter + generic timer | CSR/SBI timer path | P0 |
| MMU map/unmap/protect/translate | required | required | required | P0 |
| per-CPU base/access | required | required | required | P0 |
| SMP bootstrap | AP startup | PSCI CPU_ON | SBI HSM | P1 |
| unified ACPI/DT discovery | ACPI-centric | ACPI/DT | DT-centric | P1 |
| device model baseline | required | required | required | P1 |
| syscall skeleton | required | required | required | P1 |
| scheduler scaffolding | required | required | required | P1 |
| VFS tiny contract | required | required | required | P1 |

## Recommended Implementation Waves

### Wave 1 (P0 core contracts)
- specs `010` through `050`
- exits with stable interfaces + panic/exception consistency

### Wave 2 (P0 hardware completion)
- specs `060`, `070`, `080`
- exits with robust IRQ/timer/MMU substrate

### Wave 3 (P0/P1 multicore substrate)
- specs `090`, `100`
- exits with per-CPU + secondary CPU parking

### Wave 4 (P1 normalization and drivers)
- specs `110`, `120`
- exits with common discovery model and baseline driver framework

### Wave 5 (P1 kernel scaffolding)
- specs `130`, `140`, `150`
- exits with syscall/task/VFS skeletons for OS experimentation

### Wave 6 (discipline lock-in)
- spec `160`
- exits with CI quality gates and milestone closure

## Parity Exit Conditions

For each wave:
1. architecture-specific deferred items explicitly listed.
2. no silent `STATUS_DEFERRED` for core P0 functionality.
3. boot-time diagnostics show backend selected and ready state.
4. tests updated for all three architectures.

## First Implementation Target Order

Within each wave, prioritize:
1. x86_64 (fastest iteration)
2. arm64 (second reference architecture)
3. riscv64 (spec conformance and portability hardening)

This order optimizes debugging velocity while enforcing final parity.

## Base vs OS-Layer Boundary

- Base milestone: focus on `010` through `120`, then `160`.
- OS-layer milestone: `130`, `140`, `150`.

See [200-os-layer-next-steps.md](200-os-layer-next-steps.md) for the post-base implementation guide.
