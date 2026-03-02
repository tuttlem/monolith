# riscv64 Boot

## Producer

`arch_main(hart_id, dtb_ptr)` in `arch/riscv64/boot/main.c` constructs `boot_info_t` + `boot_info_ext_riscv64_t`, then calls `kmain`.

## Captured Entry State

- `entry_pc` via `auipc`
- `entry_sp` from `sp`
- `vm_root_table` from `satp`
- `vm_enabled` inferred from `satp != 0`

## Memory Discovery

- parses DTB memory node (`memory` or `memory@...`) when valid.
- fallback RAM range if DTB parse fails.
- inserts one normalized usable memory region (`BOOT_MEM_REGION_USABLE`).

## Additional Platform Data

- sets `boot_cpu_id` from hart id
- sets `dtb_ptr` and corresponding valid bits
- sets `serial_port` to UART base

## Extension Payload

`boot_info_ext_riscv64_t` includes:
- hart/dtb/satp snapshot
- UART base and RAM range
- entry argument copies (`a0`, `a1`)
- memory init diagnostics updated later by `arch_memory_init`
