# Arch User-Syscall Invoke Facade

Architecture-specific trap ABI glue lives in arch syscall backends, while policy code calls one neutral facade.

## Public Facade

Header: `kernel/include/arch_user_syscall.h`

- `status_t arch_user_syscall_invoke6(u64 op, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 *out_ret)`

Semantics:
- Returns `STATUS_INVALID_ARG` if `out_ret == NULL`.
- Returns `STATUS_OK` on invoke path execution and writes `*out_ret`.

## Convenience Wrappers

Header: `kernel/include/user_syscall.h`

- `user_syscall0..6(...)`

Wrappers route through `arch_user_syscall_invoke6` and zero-fill unused arguments.

## Arch Implementations

- `arch/x86_64/syscall/syscall.c`
- `arch/arm64/syscall/syscall.c`
- `arch/riscv64/syscall/syscall.c`

## HAL Selection

No runtime dispatch is required. The selected architecture build links exactly one backend implementation.
