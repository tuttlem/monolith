# Input Substrate and Arch Hooks

This substrate is a policy-neutral transport for character input.

## Scope

- In scope: queue transport, arch producer hooks, queue-first consumption helpers.
- Out of scope: shell policy, keymaps, localization, product input UX.

## Shared API

- `status_t input_init(void)`
- `status_t input_push_char(char ch)`
- `status_t input_push_char_from_irq(char ch)`
- `int input_try_pop_char(char *out_ch)`
- `u64 input_drop_count(void)`

Header: `kernel/include/input.h`

## Arch Hook Contract

- `status_t arch_input_init(const boot_info_t *boot_info)`
- `void arch_input_poll(void)`

Header: `kernel/include/arch_input.h`

Implemented per architecture:
- `arch/x86_64/input/input.c`
- `arch/arm64/input/input.c`
- `arch/riscv64/input/input.c`

## Runtime Integration

`SYSCALL_OP_INPUT_TRY_READ` uses queue-first behavior:

1. Try `input_try_pop_char`.
2. If empty, call `arch_input_poll`.
3. Retry queue pop.

If still empty, it returns `STATUS_TRY_AGAIN`.

## Validation

- `make smoke-input-x86_64`
- `make smoke-input-arm64`
- `make smoke-input-riscv64`
- `make smoke-input-matrix`
