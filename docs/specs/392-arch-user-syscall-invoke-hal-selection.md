# 392 Arch User-Syscall Invoke Helpers and HAL Selection

## Objective
Move user-mode syscall trap ABI/register glue out of policy code and into architecture syscall backends, while preserving one architecture-neutral syscall invoke surface for kernel/user-facing code.

## Multi-Architecture Closure Rule
This spec is complete only when `x86_64`, `arm64`, and `riscv64` implement the same architecture-neutral user-syscall invoke contract and pass matrix validation.

## Problem Statement
Current ToyOS/user bootstrap code carries architecture-specific trap ABI details inline (register mappings, trap instructions, clobbers/fallback behavior). This couples policy code to architecture mechanics and makes syscall API evolution harder.

## In Scope
- Define one architecture-neutral user-syscall invoke interface.
- Implement per-arch backends in:
  - `arch/x86_64/syscall/syscall.c`
  - `arch/arm64/syscall/syscall.c`
  - `arch/riscv64/syscall/syscall.c`
- Route core/user-facing wrappers (`syscall0..6` style) through that interface.
- Keep build/link selection HAL-style (arch object selection at link time).
- Add multi-arch validation targets and markers.

## Out of Scope
- New syscall operations.
- Process model redesign.
- Userspace libc ABI design.
- Non-syscall trap semantics.

## Architecture and Header Model

### Public facade (architecture-neutral)
Use a shared header in core include path, for example:

- `kernel/include/arch_user_syscall.h` (new)

```c
status_t arch_user_syscall_invoke6(BOOT_U64 op,
                                   BOOT_U64 a0,
                                   BOOT_U64 a1,
                                   BOOT_U64 a2,
                                   BOOT_U64 a3,
                                   BOOT_U64 a4,
                                   BOOT_U64 a5,
                                   BOOT_U64 *out_ret);
```

### Optional ergonomic wrappers (core)
In a core header/module, provide:

- `user_syscall0..6(...)`

All wrappers call `arch_user_syscall_invoke6(...)` and fill unused args with zero.

### Per-arch implementation location
Implement the same symbol in each arch backend file:

- `arch/x86_64/syscall/syscall.c`
- `arch/arm64/syscall/syscall.c`
- `arch/riscv64/syscall/syscall.c`

### HAL-style selection
No runtime dispatch is required. Build system already links exactly one architecture backend, so the shared symbol resolves via selected arch objects.

## Per-Architecture Backend Requirements

### x86_64
- Preserve existing trap ABI mapping and instruction (`int $0x80` in current ToyOS flow, unless Monolith standard changes).
- Document clobbers and calling convention assumptions.
- Return value semantics must match shared contract.

### arm64
- Implement backend using architecture-approved path:
  - trap path (`svc`) when ready, or
  - documented deterministic fallback path during early bring-up.
- Keep behavior explicit and testable; no silent path switching.

### riscv64
- Use `ecall` mapping consistent with shared syscall dispatcher ABI.
- Ensure return register handling matches shared contract.

## Contract Semantics
- `STATUS_OK`: invoke path executed and `*out_ret` populated.
- Non-OK status: invoke path failed, `out_ret` may be left unchanged or zeroed per implementation policy (must be documented and consistent).
- Never dereference null output pointer; return `STATUS_INVALID_ARG`.

## Migration Plan
1. Add facade header and declarations.
2. Implement `arch_user_syscall_invoke6` in all three arch syscall backends.
3. Replace inline ToyOS/user code asm glue with wrapper calls.
4. Keep old inline helper behind temporary compatibility flag only if needed.
5. Remove deprecated inline glue once matrix passes.

## Build-System Requirements
- Ensure each architecture links its backend implementation object.
- Ensure no duplicate symbol collisions in multi-target aggregate builds.
- Keep include layering strict: core code includes facade header, not arch-private headers.

## Validation Matrix
Add/extend tests so all architectures validate the same API surface:

1. Build checks:
```bash
make x86_64-uefi arm64-uefi riscv64
```

2. User-mode syscall round-trip checks:
```bash
make smoke-usermode-x86_64 smoke-usermode-arm64 smoke-usermode-riscv64
```

3. Prompt/interactive checks where applicable:
```bash
make smoke-prompt-x86_64 smoke-prompt-arm64 smoke-prompt-riscv64
```

4. Aggregate:
```bash
make usermode-matrix
```

## Acceptance Criteria
1. No policy code contains architecture-specific syscall register/instruction glue.
2. `arch_user_syscall_invoke6` is implemented in all 3 architecture syscall backends.
3. Optional `user_syscall0..6` wrappers compile and route through shared facade.
4. `usermode-matrix` passes on all supported architectures.
5. Docs clearly describe arm64 trap-vs-fallback behavior if fallback remains.

## Risks and Mitigations
- Risk: behavior drift between arch backends.
  - Mitigation: shared contract + matrix tests for return/status behavior.
- Risk: arm64 readiness differences across firmware/runtime.
  - Mitigation: explicit backend mode documentation and deterministic fallback policy.
- Risk: include-layer violations.
  - Mitigation: single core facade header and no direct core includes of arch-private headers.

## Suggested Commit Plan
1. `add arch_user_syscall facade header`
2. `implement arch_user_syscall_invoke6 in x86_64`
3. `implement arch_user_syscall_invoke6 in arm64`
4. `implement arch_user_syscall_invoke6 in riscv64`
5. `replace inline user syscall asm helpers with wrappers`
6. `matrix tests + docs`
