# 380 Trap/Syscall Register ABI Helpers (Multi-Arch)

## Objective
Remove duplicated per-arch syscall register glue by standardizing decode/encode helpers.

## Multi-Architecture Closure Rule
This spec is complete only when helper backends are implemented and used for `x86_64`, `arm64`, and `riscv64`.

## In Scope
- Arch helper to decode syscall op/args from trap frame.
- Arch helper to encode return value/status into trap frame.
- Core syscall dispatcher integration using helper API.

## Out of Scope
- New syscall operations.
- Userland ABI expansion beyond current contract.
- Architecture-agnostic dispatcher policy changes unrelated to register mapping.

## Implementation Tasks
1. Define helper contract in arch/core boundary.
2. Implement `x86_64` helper backend.
3. Implement `arm64` helper backend.
4. Implement `riscv64` helper backend.
5. Update syscall trap path to consume helper interface and remove duplicate mappings.

## Required Helper Contract (Copy Into Monolith)

```c
typedef struct {
  BOOT_U64 op;
  BOOT_U64 args[6];
} syscall_abi_frame_t;

status_t arch_syscall_decode(const void *trap_frame, syscall_abi_frame_t *out);
status_t arch_syscall_encode_ret(void *trap_frame, BOOT_U64 value);
```

## Register Mapping Targets

- `x86_64`: `rax=op`, `rdi/rsi/rdx/r10/r8/r9=args`, `rax=ret`.
- `arm64`: `x8=op`, `x0..x5=args`, `x0=ret`.
- `riscv64`: `a7=op`, `a0..a5=args`, `a0=ret`.

## Acceptance Criteria
1. Syscall trap logic no longer duplicates register mapping at call sites.
2. Helpers exist and are active on all three architectures.
3. Unknown/invalid syscall paths fail safely with deterministic status.
