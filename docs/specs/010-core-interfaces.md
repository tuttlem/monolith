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
3. Use fixed-width semantics via existing BOOT types or explicit typedefs.
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
