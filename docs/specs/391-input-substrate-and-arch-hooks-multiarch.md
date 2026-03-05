# 391A Input Substrate and Arch Hooks (Multi-Arch, Monolith Scope)

## Objective
Provide a minimal, architecture-neutral input substrate in Monolith, with architecture hook points and validation 
scaffolding, without forcing OS policy decisions.

## Multi-Architecture Closure Rule
Complete only when `x86_64`, `arm64`, and `riscv64` build and pass shared input-substrate validation hooks.

## Monolith Scope (In)
- Core input contract and queue abstraction.
- Architecture hook interface for input producer backends.
- Syscall/runtime integration hook points (queue-first capability), policy-neutral.
- Build wiring and smoke matrix coverage.
- Optional reference drivers explicitly marked bring-up baseline.

## Monolith Scope (Out)
- Final OS keyboard policy (priority/order semantics).
- Shell/CLI line-editing behavior.
- Production HID/keymap/localization policy.
- Product-level driver strategy decisions.

## API Contract
Add shared headers:

- `kernel/include/input.h`
- `kernel/include/arch_input.h`

Suggested minimal API:

```c
status_t input_init(void);
status_t input_push_char(char ch);
status_t input_push_char_from_irq(char ch);
int input_try_pop_char(char *out_ch);
BOOT_U64 input_drop_count(void);

status_t arch_input_init(const boot_info_t *boot_info);
void arch_input_poll(void);
```

Contract notes:
- Core queue is policy-neutral transport from producer to consumer.
- No shell semantics embedded in substrate.
- Keep API narrow and deterministic.

## Architecture Backend Placement
Implement backend hooks in architecture trees:

- `arch/x86_64/...`
- `arch/arm64/...`
- `arch/riscv64/...`

Reference backend choices may differ by architecture (for example PS/2 vs UART) and are allowed as bring-up defaults.

## Syscall/Runtime Integration Requirements
- Add queue-first consumption hook in input-read path.
- Preserve fallback compatibility path where existing substrate requires it.
- Keep user-pointer safety unchanged (`copy_to_user_checked`, etc.).

## Build and Test Requirements
Add/maintain:
- `smoke-input-x86_64`
- `smoke-input-arm64`
- `smoke-input-riscv64`
- `smoke-input-matrix`

Include these in aggregate usermode matrix.

Required markers:
- input backend ready marker per architecture
- prompt/usermode marker
- no panic/exception markers

## Acceptance Criteria
1. Shared input substrate API exists and is used by all architectures.
2. All three architecture backends compile and initialize through shared hooks.
3. Input queue-first path is functional without regressions to fallback path.
4. `smoke-input-matrix` and `usermode-matrix` pass tri-arch.
5. Documentation clearly states these are substrate/reference mechanisms, not final OS policy.

## Suggested Commit Plan
1. `input substrate API + queue`
2. `arch input hook interface + per-arch backend stubs`
3. `queue-first runtime integration`
4. `build + smoke-input matrix`
5. `docs: substrate boundaries`
