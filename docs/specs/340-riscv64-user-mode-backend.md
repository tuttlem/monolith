# 340 riscv64 User-Mode Backend

## Objective
Implement the `riscv64` backend for the architecture-neutral `arch_user_mode_*` contract.

## Multi-Architecture Closure Rule
This spec is part of a tri-arch program and is only globally complete when corresponding `x86_64` and `arm64` specs are also complete.

## In Scope
- `arch_user_mode_set_kernel_stack(...)` backend behavior on `riscv64`.
- `arch_user_mode_enter(...)` backend behavior on `riscv64`.
- S-mode->U-mode entry and U-mode->S-mode `ecall` return compatibility.

## Out of Scope
- Shell command policy.
- Scheduler/process expansion.
- Non-`riscv64` backend implementation details.

## Implementation Tasks
1. Implement riscv64 user entry setup:
   - set user PC/stack
   - set privilege return state for U-mode
2. Implement kernel return stack/context wiring aligned with trap entry model.
3. Ensure `ecall` from U-mode is recognized and dispatched as syscall.
4. Ensure syscall arg/return register mapping matches shared dispatcher ABI.
5. Verify deterministic return to U-mode (or defined exit policy).

## Backend Skeleton (riscv64)

```c
status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  g_riscv64_kernel_stack_top = (u64)(uptr)kernel_stack_top;
  return STATUS_OK;
}

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, u64 user_sp) {
  u64 sepc = (u64)(uptr)entry;
  u64 sstatus;

  __asm__ volatile("csrr %0, sstatus" : "=r"(sstatus));
  sstatus &= ~(1ULL << 8); /* clear SPP => return to U-mode */

  __asm__ volatile(
      "mv sp, %0\n\t"
      "mv a0, %1\n\t"
      "csrw sepc, %2\n\t"
      "csrw sstatus, %3\n\t"
      "sret\n\t"
      :
      : "r"(user_sp), "r"(arg), "r"(sepc), "r"(sstatus)
      : "memory");
  __builtin_unreachable();
}
```

Trap integration rule:
- `ecall` from U-mode must dispatch through shared syscall path and resume U-mode unless exit/fault policy triggers.

## Required Invariants
- User mode runs in U privilege, kernel in S privilege.
- Trap handler preserves context required for resume.
- Unsupported/invalid entry states return deterministic errors.

## Validation Commands
```bash
make riscv64
make smoke-riscv64
make run-riscv64
```

Recommended serial-log validation:
```bash
QEMU_HEADLESS=1 QEMU_SERIAL="file:build/riscv64/usermode.log" timeout 60s ./scripts/run-qemu.sh riscv64
tail -n 200 build/riscv64/usermode.log
```

## Acceptance Criteria
1. `riscv64` backend compiles and links through generic `arch_user_mode_*` API.
2. System enters U-mode and services syscall traps in S-mode.
3. No trap-loop or context corruption on basic syscall round-trip.
4. riscv64 smoke tests pass with user-mode markers present in log.
