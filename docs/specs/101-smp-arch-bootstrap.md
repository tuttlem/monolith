# 101 SMP Architecture Bootstrap Backends

## Goal

Replace no-op SMP architecture stubs with concrete backend behavior and explicit progress signals.

## Scope

- x86_64 + arm64: use UEFI MP Services protocol when available
- riscv64: discover possible CPUs from DTB and report deferred start path
- keep failure/reporting semantics explicit via `status_t`

## x86_64 / arm64 Requirements

- locate MP Services protocol through UEFI Boot Services
- query processor counts
- attempt AP startup callback path
- publish started count and return:
  - `STATUS_OK` when at least one AP entry is reached
  - `STATUS_DEFERRED` when protocol unavailable
  - `STATUS_TRY_AGAIN` on hard bootstrap failure

## riscv64 Requirements

- parse DTB CPU nodes to estimate possible CPU count
- return `STATUS_DEFERRED` until SBI HSM start path is implemented

## Acceptance Criteria

- architecture backends perform real platform interaction (not constant stubs)
- `smp_cpu_count_possible()` reflects discovered topology where available
- boot remains stable on all active architectures
