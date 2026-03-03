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
| bus core + PCI/USB enumeration | required | required | required | P1 |
| standard capability profiles/gating | required | required | required | P1 |
| storage/input/display baseline domains | required | required | required | P1 |
| network baseline domain | recommended | recommended | recommended | P2 |
| audio baseline domain | optional | optional | optional | P2 |
| syscall skeleton | required | required | required | P1 |
| hardware resource manager | required | required | required | P0 |
| IRQ domains + MSI abstraction | required | required | required | P0 |
| DMA mapping API | required | required | required | P0 |
| IOMMU subsystem | recommended | recommended | recommended | P1 |
| clock/reset framework | recommended | required | required | P1 |
| CPU context facilities | required | required | required | P1 |
| execution personality hooks | required | required | required | P1 |
| driver helper kits | required | required | required | P1 |
| observability substrate | required | required | required | P1 |
| time system completion | required | required | required | P0 |
| SMP completion | recommended | recommended | recommended | P1 |
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

### Wave 5 (P1 device usability domains)
- specs `121` through `129`
- exits with bus-level enumeration plus profile-based optional domains

### Wave 6 (P1 kernel scaffolding)
- specs `130`, `131` through `139`
- exits with policy-neutral execution/runtime substrate and reusable driver primitives

### Wave 7 (P0/P1 convergence hardening)
- specs `141`, `142`, `143`
- exits with observability + time quality + multicore substrate closure

### Wave 8 (OS-facing scaffolding, optional for substrate milestone)
- specs `140`, `150`
- exits with syscall/task/VFS skeletons for OS experimentation

### Wave 9 (discipline lock-in)
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

- Base milestone: focus on `010` through `139`, then `141`, `142`, `143`, then `160`.
- OS-layer milestone: `140`, `150` (plus downstream-specific evolution).

See [200-os-layer-next-steps.md](200-os-layer-next-steps.md) for the post-base implementation guide.
