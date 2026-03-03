# 390 Integration Validation Matrix and Smoke Targets (Multi-Arch)

## Objective
Provide deterministic validation for the Monolith user-mode substrate APIs across all supported architectures.

## Multi-Architecture Closure Rule
This spec is complete only when matrix validation passes for `x86_64`, `arm64`, and `riscv64`.

## In Scope
- Cross-arch compile checks.
- Per-arch user-mode entry and syscall round-trip smoke checks.
- Negative tests for bad user-memory access and unsupported paths.
- Log markers for pass/fail triage.

## Out of Scope
- Full conformance test suite for advanced process semantics.
- Performance benchmarking.
- Non-deterministic/manual-only test steps.

## Implementation Tasks
1. Add/update make targets for substrate integration checks.
2. Add/extend smoke scripts with marker assertions.
3. Add scripted syscall interaction flow for each architecture where input path exists.
4. Document expected output markers and failure signatures.

## Required Matrix Target (Copy Into Monolith)

```bash
make usermode-matrix
```

Expected expansion:

```bash
make x86_64-uefi arm64-uefi riscv64
make smoke-x86_64 smoke-arm64 smoke-riscv64
make smoke-usermode-x86_64 smoke-usermode-arm64 smoke-usermode-riscv64
```

## Marker Requirements
- boot marker per arch: `Starting Monolith (<arch>)`
- user-entry marker: `usermode: launching init task` (or project-defined equivalent)
- no-fault markers: absence of `interrupt: unhandled` / `exception:`

## Script Pattern

```bash
set +e
QEMU_SERIAL="file:${log}" timeout 50s ./scripts/run-qemu.sh "${arch}" -display none -monitor none >/dev/null 2>&1
rc=$?
set -e
[[ ${rc} -eq 0 || ${rc} -eq 124 ]] || fail
tr -d '\r' <"${log}" | grep -Fq "${boot_marker}" || fail
tr -d '\r' <"${log}" | grep -Fq "${user_marker}" || fail
```

## Suggested Baseline Commands
```bash
make x86_64-uefi arm64-uefi riscv64
make smoke-x86_64 smoke-arm64 smoke-riscv64
```

## Acceptance Criteria
1. One command runs full tri-arch substrate validation matrix.
2. Positive and negative paths are covered per architecture.
3. Failure logs are deterministic and actionable without interactive debugging.
