# 130 Syscall Transport ABI (Non-Opinionated)

## Goal

Define a stable, cross-architecture syscall call interface that gives OS developers a consistent entry/dispatch boundary without imposing POSIX or any specific syscall semantic model.

## Scope

- Versioned syscall transport contract (`abi_version`, `op`, args, return tuple).
- Architecture-specific trap entry glue (`syscall`/`svc`/`ecall`) into one generic dispatcher.
- Namespace-aware operation dispatch (`core` reserved range + OS-defined extension ranges).
- Registration hooks for downstream OS syscall providers.
- Minimal substrate diagnostics calls only (no POSIX policy in base).

## Non-Goals

- Do not define a mandatory POSIX syscall set (`open/read/write/fork/...`) in the substrate.
- Do not force one object model, file model, process model, or capability model.
- Do not lock downstream OS implementations into one userspace protocol.

## Required Components

1. Per-arch entry glue
  - x86_64: syscall trap entry shim
  - arm64: SVC entry shim
  - riscv64: ECALL entry shim
2. Generic syscall frame
  - normalized view of `abi_version`, `op`, `arg0..argN`, caller context
3. Generic dispatcher
  - validates ABI version
  - routes by operation namespace/id
  - returns deterministic `status_t` + value tuple
4. Registration model
  - base supports handler registration per op/namespace
  - OS layer installs its own policies and semantics
5. Deterministic fallback behavior
  - unknown op/namespace -> explicit not-supported/not-found status
  - malformed call -> explicit invalid-arg/status fault policy

## ABI Design Rules

- Explicit calling convention per architecture must be documented.
- Transport ABI versioning is mandatory and checked on every call path.
- Stable numeric assignments are guaranteed only for substrate-reserved ops.
- Extension ranges are intentionally delegated to downstream OS designs.
- Return contract uses substrate `status_t` plus optional value payload.
- Userspace-visible errno/POSIX mapping, if desired, is an OS-layer adapter concern.

## Operation Numbering Model

- Reserve a small substrate core range for base services:
  - `0x0000-0x00FF`: substrate transport/meta ops only
- Keep the primary syscall space OS-defined:
  - `0x0100-0x7FFF`: downstream OS syscall space (including POSIX-style layouts if desired)
- Reserve a high range for optional experimental/vendor extensions:
  - `0x8000-0xFFFF`: experimental/vendor space
- Document reserved vs free ranges clearly so OS developers can evolve safely.

## Minimal Substrate Call Set

Provide only neutral bootstrap calls, for example:
- `sys_abi_info` (query transport ABI version/features)
- `sys_debug_log` (best-effort kernel diagnostics channel)
- `sys_time_now` (monotonic time query)

Suggested reserved IDs inside substrate range:
- `0x0001`: `sys_abi_info`
- `0x0002`: `sys_debug_log`
- `0x0003`: `sys_time_now`

These are substrate diagnostics/utilities, not policy-defining OS syscalls.

## Acceptance Criteria

- Syscall entry path executes on `x86_64`, `arm64`, and `riscv64`.
- ABI version mismatch returns deterministic status.
- Unknown operation returns deterministic status.
- Substrate reserved calls are stable and documented.
- OS-defined extension calls can be registered without touching core transport ABI.
- Manual documentation is updated alongside implementation changes (API docs + per-arch entry mapping).
