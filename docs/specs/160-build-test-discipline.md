# 160 Build/Test and Bring-up Discipline

## Goal

Guarantee reproducible progress and parity through automated checks.

## Test Layers

1. compile/build checks per architecture
2. smoke boot tests (serial log assertions)
3. subsystem self-tests (allocator, timer, exception path)
4. architecture-agnostic unit tests where feasible

## Required CI Matrix

- `x86_64`: build + boot smoke + timer + exception decode
- `arm64`: build + boot smoke + timer + exception decode
- `riscv64`: build + boot smoke + interrupt path + timer status expectation

## Artifact Requirements

- archived serial logs per architecture
- parsed PASS/FAIL markers for self-tests
- stable command entry points under `scripts/`

## Quality Gates Per Phase

Each spec phase can merge only when:
1. docs updated
2. all matrix targets pass or documented deferred exceptions exist
3. no regression in earlier phase tests

## Milestone Definition: "Usable Base"

The base is considered complete when:
- stable interfaces frozen
- panic/fault path standardized
- clocksource+clockevent available
- mm map/unmap/protect usable
- per-CPU and SMP skeleton functional
- discovery and device model baseline in place
- syscall/scheduler/VFS skeleton integrated
- CI matrix green for all in-scope architectures
