# Syscall Transport ABI

This layer provides a stable syscall call interface without imposing OS policy.

## Intent

- Keep syscall transport and dispatch consistent across architectures.
- Reserve only a small substrate range for bootstrap utilities.
- Leave semantic syscall design (POSIX-like or not) to downstream OS developers.

## Numbering Contract

- Substrate reserved: `0x0000-0x00FF`
- OS-defined primary range: `0x0100-0x7FFF`
- Vendor/experimental range: `0x8000-0xFFFF`

Current substrate IDs:
- `0x0001`: `sys_abi_info`
- `0x0002`: `sys_debug_log`
- `0x0003`: `sys_time_now`

## Public API

Header: `kernel/include/syscall.h`

- `status_t syscall_init(const boot_info_t *boot_info)`
- `status_t syscall_register(u64 op, syscall_handler_t handler, const char *owner)`
- `status_t syscall_dispatch(const syscall_request_t *req, syscall_response_t *resp)`
- `status_t syscall_invoke_kernel(..., syscall_response_t *resp)`
- `status_t syscall_invoke_trap(..., syscall_response_t *resp)`
- `u64 syscall_abi_info_word(void)`
- `int syscall_trap_entry_ready(void)`
- `void syscall_dump_table(void)`

Architecture hook:
- `status_t arch_syscall_init(const boot_info_t *boot_info)` in `kernel/include/arch_syscall.h`
- `status_t arch_syscall_get_vector(u64 *out_vector)` in `kernel/include/arch_syscall.h`
- `status_t arch_syscall_trigger(void)` in `kernel/include/arch_syscall.h`

## Current Phase Behavior

- Generic transport dispatcher is fully active.
- Substrate handlers are registered at init:
  - ABI info query
  - debug log channel
  - monotonic time query
- Architecture trap entry glue is active on all supported architectures:
  - `x86_64`: `int $0x80` -> vector `0x80`
  - `arm64`: `svc #0` -> synthetic syscall vector `64` from synchronous trap decode
  - `riscv64`: SSIP-triggered trap path to vector `64` with deterministic early-boot fallback dispatch
- `syscall_invoke_kernel` and `syscall_invoke_trap` both dispatch through the same registered handlers.

## ABI Info Encoding

`syscall_abi_info_word()` packs:
- bits `0..15`: transport ABI version
- bits `16..23`: active architecture ID
- bits `24..39`: feature bits (`SYSCALL_ABI_FEATURE_*`)

## Example

```c
syscall_response_t r;
status_t st = syscall_invoke_trap(SYSCALL_OP_TIME_NOW, 0, 0, 0, 0, 0, 0, &r);
if (st == STATUS_OK && r.status == STATUS_OK) {
  kprintf("sys_time_now=%llu\n", r.value);
}
```
