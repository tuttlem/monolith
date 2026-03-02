# 000 Foundation and Rules

## Scope Boundary

This project layer is the hardware substrate and early-kernel base only.

In scope:
- CPU/interrupt/timer/mmu abstractions
- architecture backend integration
- discovery and normalization of hardware description sources
- basic execution substrate for later kernel subsystems

Out of scope (for this phase):
- full process model
- full VFS implementation
- production security model
- complex VM policy

## Architectural Rule: Policy Above Mechanism

- Mechanism lives in substrate modules and architecture backends.
- Policy lives in later kernel layers built on top.
- No architecture-specific policy logic in generic modules.

## Interface Stability Rule

For all interfaces under `kernel/include` and new `kernel/subsys/*` headers:
1. versioned contracts
2. documented return semantics (`status_t`)
3. explicit ownership/lifetime rules
4. no backend internals leaked to callers

## Cross-Architecture Parity Rule

Each phase must define:
- required behavior for all three architectures
- allowed `STATUS_DEFERRED` paths (temporary only)
- parity exit criteria

No phase is complete unless parity criteria are met or deferred scope is explicitly accepted with rationale.

## Bring-up Invariants

1. Kernel entry always receives `boot_info_t`.
2. All optional boot fields are guarded by valid-mask bits.
3. Generic kernel code does not inspect architecture registers directly.
4. Panic/fault path is always callable from early bring-up.

## Deliverable Requirements for Every Spec

Each implementation spec must provide:
- C APIs and types
- init ordering
- architecture backend obligations
- diagnostics/logging requirements
- failure model
- test matrix
