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

## Foundation Deliverables (This Phase)

`000-foundation` is complete only when the repository has:
1. A documented phase gate checklist used by all specs.
2. A reusable implementation template for future spec branches.
3. A strict base-vs-OS-layer boundary recorded in specs.

These are provided by:
- `docs/specs/FOUNDATION-GATES.md`
- `docs/specs/SPEC-IMPLEMENTATION-TEMPLATE.md`
- `docs/specs/170-roadmap-by-arch.md`
- `docs/specs/200-os-layer-next-steps.md`

## Branch Workflow Contract

For each spec branch:
1. Branch name matches spec file name (`040-arch-cpu`, etc.).
2. Changes are limited to that spec's scope.
3. Build + test are run before merge.
4. Merge back to `master` only on green checks, or with explicit deferred rationale.

## Definition of Done for `000-foundation`

- Foundation rules are explicit and discoverable from `docs/specs/README.md`.
- Every future spec can be executed with one standard template and one standard gate checklist.
- Base milestone and OS-layer milestones are clearly separated.
