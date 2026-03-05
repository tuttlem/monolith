# 330 arm64 User-Mode Backend

## Objective
Implement the `arm64` backend for the architecture-neutral `arch_user_mode_*` contract.

## Multi-Architecture Closure Rule
This spec is part of a tri-arch program and is only globally complete when corresponding `x86_64` and `riscv64` specs are also complete.

## In Scope
- `arch_user_mode_set_kernel_stack(...)` backend behavior on `arm64`.
- `arch_user_mode_enter(...)` backend behavior on `arm64`.
- EL1->EL0 entry and EL0->EL1 syscall/trap return compatibility.

## Out of Scope
- User shell policy.
- Filesystem/process model changes.
- Non-`arm64` backend implementation details.

## Implementation Tasks
1. Implement arm64 user entry setup:
   - target EL0 execution state
   - initial `ELR_EL1` and `SPSR_EL1` policy
   - user SP setup
2. Implement kernel return stack wiring needed by trap handling model.
3. Ensure `svc` path is correctly classified as user syscall in interrupt/trap flow.
4. Ensure syscall argument/return register ABI matches shared dispatcher expectations.
5. Verify return to EL0 after syscall (or defined task-exit path) is stable.

## Backend Skeleton (arm64)

Define the generic backend in arch code:

```c
status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  /* If SP_EL1 is managed per-task, store top for trap return path. */
  g_arm64_kernel_stack_top = (u64)(uptr)kernel_stack_top;
  return STATUS_OK;
}

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, u64 user_sp) {
  u64 elr = (u64)(uptr)entry;
  u64 spsr = 0; /* EL0t, interrupts policy per kernel rules */
  __asm__ volatile(
      "msr sp_el0, %0\n\t"
      "mov x0, %1\n\t"
      "msr elr_el1, %2\n\t"
      "msr spsr_el1, %3\n\t"
      "eret\n\t"
      :
      : "r"(user_sp), "r"(arg), "r"(elr), "r"(spsr)
      : "x0", "memory");
  __builtin_unreachable();
}
```

User trap classification requirement:
- synchronous exception with `EC=0x15` (`svc`) maps to syscall vector.

## Required Invariants
- User enters at EL0, kernel remains EL1.
- Trap path does not corrupt saved execution context.
- Invalid user entry state fails with deterministic status.

## Validation Commands
```bash
make arm64-uefi
make smoke-arm64
make run-arm64
```

Recommended non-interactive log check:
```bash
QEMU_SERIAL="file:build/arm64/usermode.log" timeout 50s ./scripts/run-qemu.sh arm64 -display none -monitor none
tail -n 200 build/arm64/usermode.log
```

## Acceptance Criteria
1. `arm64` backend compiles and links through generic `arch_user_mode_*` API.
2. System enters EL0 user context and returns to EL1 via syscall/trap path.
3. No kernel crash on normal user syscall path.
4. arm64 smoke tests pass with user-mode markers present in log.
