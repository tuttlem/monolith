# 125 Device Reporting and Introspection

## Goal

Expose a stable introspection/reporting interface so developers can inspect all discovered hardware consistently across architectures.

## Scope

- Add device report APIs:
  - enumerate all devices
  - filter by class/bus
  - fetch resources and capability metadata
- Add human-readable dump format for bring-up logs.
- Add machine-friendly struct-based report path for kernel subsystems.
- Add smoke checks validating expected core classes on each supported architecture.

## Non-Goals

- Userspace tooling ABI freeze.
- Security/authorization model for report consumers.

## Acceptance Criteria

- One command/path prints complete hardware inventory (bus tree + resources).
- Same core report format works on x86_64, arm64, riscv64.
- Missing/unavailable buses are explicitly represented, not silently omitted.
