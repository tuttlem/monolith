# 320 x86_64 User-Mode Backend

## Objective
Implement the `x86_64` backend for the architecture-neutral `arch_user_mode_*` contract.

## Multi-Architecture Closure Rule
This spec is part of a tri-arch program and is only globally complete when corresponding `arm64` and `riscv64` specs are also complete.

## In Scope
- `arch_user_mode_set_kernel_stack(...)` backend behavior on `x86_64`.
- `arch_user_mode_enter(...)` backend behavior on `x86_64`.
- Correct user->kernel return path compatibility with syscall/trap flow.

## Out of Scope
- Shell or userspace command policy.
- Scheduler policy changes.
- Non-`x86_64` backend implementation details.

## Implementation Tasks
1. Map existing x86 user-mode mechanisms to the new generic API.
2. Ensure kernel return stack setup uses architecture-correct mechanism (TSS/RSP0 path or equivalent current backend path).
3. Preserve ring transition invariants:
   - user entry at CPL3
   - kernel handler at CPL0
4. Validate syscall return path after user entry remains stable.
5. Remove duplicate legacy entry points or wrap them behind the generic API.

## Reference Implementation Pattern (x86_64)

Use wrappers so existing proven code can be lifted directly:

```c
status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  return x86_64_user_mode_set_kernel_stack(kernel_stack_top);
}

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, BOOT_U64 user_sp) {
  x86_64_user_mode_enter(entry, arg, user_sp);
}
```

User syscall stub pattern:

```c
static BOOT_U64 user_syscall6(BOOT_U64 op, BOOT_U64 a0, BOOT_U64 a1, BOOT_U64 a2, BOOT_U64 a3, BOOT_U64 a4, BOOT_U64 a5) {
  register BOOT_U64 rax __asm__("rax") = op;
  register BOOT_U64 rdi __asm__("rdi") = a0;
  register BOOT_U64 rsi __asm__("rsi") = a1;
  register BOOT_U64 rdx __asm__("rdx") = a2;
  register BOOT_U64 r10 __asm__("r10") = a3;
  register BOOT_U64 r8  __asm__("r8")  = a4;
  register BOOT_U64 r9  __asm__("r9")  = a5;
  __asm__ volatile("int $0x80" : "+a"(rax) : "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
  return rax;
}
```

## Required Invariants
- Kernel stack pointer configured before user entry.
- User stack pointer is canonical and aligned.
- Trap/syscall path returns deterministically without register corruption.

## Validation Commands
```bash
make x86_64-uefi
make smoke-x86_64
make smoke-prompt-x86_64
```

Optional scripted interaction check:
```bash
{ sleep 8; printf 'help\n'; sleep 2; printf 'echo hi\n'; sleep 2; } | \
timeout 40s ./scripts/run-qemu.sh x86_64 -display none -monitor none
```

## Acceptance Criteria
1. `x86_64` backend compiles and links through generic `arch_user_mode_*` API.
2. Boot reaches user-mode prompt path and remains interactive.
3. Syscall round-trip from user mode returns correctly.
4. No regression in existing x86 smoke tests.
