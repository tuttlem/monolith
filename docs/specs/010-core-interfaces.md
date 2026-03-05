# 010 Stable Core Interfaces and Contracts

## Goal

Freeze minimal HAL-style interfaces so all future kernel work targets stable generic APIs.

## Interface Families

Required core families:
- `arch_cpu`
- `arch_irq`
- `arch_timer`
- `arch_mm`

Related generic services:
- `status`/panic/assert
- `time`
- `irq core`
- `mm map`

## Proposed Header Layout

- `kernel/include/arch_cpu.h`
- `kernel/include/arch_irq.h`
- `kernel/include/arch_timer.h` (existing, to evolve)
- `kernel/include/arch_mm.h`
- `kernel/include/panic.h`
- `kernel/include/timebase.h`
- `kernel/include/mm_map.h`

## Interface Design Rules

1. Generic types first.
2. Opaque handles where backend state is needed.
3. Use fixed-width semantics via common kernel types (`u64`, `u32`, `uptr`, etc.) and reserve BOOT types for boot ABI boundary structs.
4. `status_t` for fallible calls.
5. No global mutable state exposed through headers.

## Versioning Rule

Every exported core interface header must define:
- major/minor API version constants
- compatibility statement

Example pattern:
- breaking change: major bump
- additive extension: minor bump

## Generic Init Sequencing Contract

Target sequence after boot handoff:
1. `arch_cpu_early_init`
2. `arch_mm_early_init`
3. generic memory allocators
4. `irq_init`
5. `time_init`
6. discovery and device framework
7. syscall/task/VFS scaffolding

## Acceptance Criteria

- all generic kernel modules compile without architecture-specific includes
- all architecture backends compile against same header signatures
- documented contract for every public function

## Implementation Notes (Current Repository)

`010-core-interfaces` is implemented with a compatibility-preserving freeze:
- `kernel/include/arch_cpu.h` (new stable CPU interface + version constants)
- `kernel/include/arch_irq.h` (new stable IRQ interface + version constants)
- `kernel/include/arch_mm.h` (new stable MM early-init interface + version constants)
- `kernel/include/arch_timer.h` (version constants added)
- `kernel/include/arch_interrupts.h` retained as compatibility header

Generic kernel callsites now target stable names:
- `kmain` uses `arch_cpu_early_init` and `arch_mm_early_init`
- interrupt core uses `arch_irq_*`

Backend files keep existing `arch_interrupts_*` and `arch_memory_init` symbols for now.
This avoids churn and preserves behavior while freezing generic interface contracts.
