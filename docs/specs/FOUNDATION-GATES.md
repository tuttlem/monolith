# Foundation Phase Gates

Use this checklist before merging any spec branch into `master`.

## 1) Scope Gate

- Branch name matches spec id/name.
- Changes stay inside that spec's documented scope.
- Any out-of-scope work is deferred or split into a follow-up spec branch.

## 2) Interface Gate

- Public headers are documented.
- API changes include versioning/compatibility notes where applicable.
- Ownership/lifetime/error semantics are explicit.

## 3) Cross-Architecture Gate

- `x86_64`, `arm64`, `riscv64` behavior is documented for the changed subsystem.
- Any temporary `STATUS_DEFERRED` path is explicitly justified.
- No architecture-specific internals leak into generic kernel interfaces.

## 4) Diagnostics Gate

- Bring-up logs and failure reporting are clear.
- Panic/fault/exception paths remain reachable and informative.
- Debug toggles are controlled by central config policy.

## 5) Validation Gate

- Build completes for all targeted architectures that are expected to pass in this environment.
- Relevant smoke/self-tests run and results are recorded in the merge summary.
- Known environment limitations are explicitly listed when a test cannot run.

## 6) Merge Gate

- Commit message is spec-scoped.
- Merge to `master` only with passing checks or an explicit documented exception.
- Follow-up tasks are captured in specs/docs, not left implicit.
