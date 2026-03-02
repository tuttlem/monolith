# Boot ABI (`boot_info_t`)

Canonical ABI definition: `kernel/include/boot_info.h`

## Versioning

- `BOOT_INFO_ABI_VERSION` is currently `2`.
- `boot_info.abi_version` must match the kernel's expected ABI.

## Architecture IDs

- `BOOT_INFO_ARCH_X86_64`
- `BOOT_INFO_ARCH_ARM64`
- `BOOT_INFO_ARCH_RISCV64`

## Valid-Mask Strategy

`boot_info.valid_mask` advertises which optional fields are valid (bit flags like `BOOT_INFO_HAS_MEMMAP`, `BOOT_INFO_HAS_FRAMEBUFFER`, etc.).

Rules:
1. Producer sets bits only for populated fields.
2. Consumer checks bits before trusting optional fields.
3. Core code should remain portable by relying on mask checks, not architecture assumptions.

## `boot_info_t` Field Guide

### ABI identity
- `abi_version`: handoff ABI version.
- `arch_id`: producing architecture.
- `valid_mask`: optional-field bitmap.

### Entry state
- `entry_pc`: instruction pointer at handoff.
- `entry_sp`: stack pointer at handoff.
- `vm_enabled`: virtual memory enabled at entry.
- `vm_root_table`: root page-table register value (`cr3`, `ttbr0`, `satp`, architecture-specific encoding).

### Firmware pointers
- `uefi_system_table`, `uefi_configuration_table`: valid for UEFI paths.

### Memory map stream + normalized regions
- `memory_map`, `memory_map_size`, `memory_map_descriptor_size`, `memory_map_descriptor_version`: raw firmware memory map descriptor stream metadata.
- `memory_region_count`, `memory_region_capacity`, `memory_regions[]`: normalized memory regions for portable consumers.

### Platform tables and CPU identity
- `acpi_rsdp`: ACPI RSDP pointer (if available).
- `dtb_ptr`: DTB pointer (if available).
- `boot_cpu_id`: boot CPU identifier.

### Architecture extension payload
- `arch_data_ptr`, `arch_data_size`: pointer/size to arch-specific extension struct.

### Early console outputs
- framebuffer fields: for early graphics console metadata.
- `serial_port`: base I/O/MMIO address for early serial.

## Normalized Memory Region Kinds

- `BOOT_MEM_REGION_USABLE`
- `BOOT_MEM_REGION_RESERVED`
- `BOOT_MEM_REGION_ACPI_RECLAIM`
- `BOOT_MEM_REGION_ACPI_NVS`
- `BOOT_MEM_REGION_MMIO`

## Extension Structs

### `boot_info_ext_uefi_t`
Used by UEFI-based x86_64 and arm64.

Important members:
- firmware handles/pointers (`image_handle`, `boot_services`, `runtime_services`)
- memory-init/paging diagnostics:
  - `mem_init_status`
  - `mem_old_root`, `mem_new_root`
  - `mem_mapped_bytes`
  - `paging_old_cr3`, `paging_new_cr3`, `paging_identity_bytes` (names kept generic but x86-biased)

### `boot_info_ext_riscv64_t`
Used by riscv64 boot path.

Important members:
- boot metadata (`hart_id`, `dtb_ptr`, `entry_a0`, `entry_a1`)
- address-space and platform (`satp`, `uart_base`, `ram_base`, `ram_size`)
- memory-init diagnostics (`mem_init_status`, roots, mapped bytes)

## Runtime Diagnostics

`kernel/diag/boot_info.c` prints:
- common `boot_info_t` fields
- a preview of memory regions
- extension-specific fields (`uefi_ext` or `riscv_ext`)

Use `diag_boot_info_print(boot_info)` from `kmain` for bring-up validation.
