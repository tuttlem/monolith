# Boot Info ABI

This document defines the kernel boot input ABI frozen in `BOOT_INFO_ABI_VERSION = 2`.

All kernel entry paths now pass exactly one input:

```c
void kmain(const boot_info_t *boot_info);
```

The struct is defined in `kernel/include/boot_info.h`.

## Field Contract

- `abi_version`: ABI version. Must be `BOOT_INFO_ABI_VERSION`.
- `arch_id`: Architecture id (`BOOT_INFO_ARCH_*`).
- `valid_mask`: Bitmask describing which fields are valid (`BOOT_INFO_HAS_*`).
- `entry_pc`: Entry program counter snapshot.
- `entry_sp`: Entry stack pointer snapshot.
- `vm_enabled`: `1` if virtual memory/paging is enabled at entry.
- `vm_root_table`: Active top-level VM root table (for x86_64 this is `CR3`).
- `uefi_system_table`: Raw pointer to `EFI_SYSTEM_TABLE` when booted via UEFI.
- `uefi_configuration_table`: Raw pointer to UEFI configuration table array.
- `memory_map`: Pointer to firmware memory map descriptor stream.
- `memory_map_size`: Total bytes at `memory_map`.
- `memory_map_descriptor_size`: Size of one memory descriptor.
- `memory_map_descriptor_version`: Descriptor format version.
- `acpi_rsdp`: Pointer to ACPI RSDP if present, else `0`.
- `dtb_ptr`: Pointer to Device Tree Blob if present, else `0`.
- `boot_cpu_id`: Boot CPU/hart id if known, else `0`.
- `arch_data_ptr`: Pointer to architecture/firmware-specific extension payload.
- `arch_data_size`: Size in bytes of the extension payload at `arch_data_ptr`.
- `framebuffer_base`: Framebuffer base address if discovered, else `0`.
- `framebuffer_width`: Framebuffer width in pixels.
- `framebuffer_height`: Framebuffer height in pixels.
- `framebuffer_pixels_per_scanline`: Framebuffer pitch in pixels.
- `framebuffer_format`: Platform-defined pixel format enum value.
- `serial_port`: Serial base or handle if known, else `0`.

## x86_64 UEFI Population (Current)

In `arch/x86_64/boot/efi_main.c`, these fields are actively populated:

- `abi_version`
- `arch_id = BOOT_INFO_ARCH_X86_64`
- `valid_mask` includes:
  - `BOOT_INFO_HAS_ENTRY_STATE`
  - `BOOT_INFO_HAS_VM_STATE`
  - `BOOT_INFO_HAS_UEFI_SYSTEM_TABLE`
  - `BOOT_INFO_HAS_UEFI_CONFIG_TABLE`
  - `BOOT_INFO_HAS_ARCH_DATA`
  - `BOOT_INFO_HAS_ACPI_RSDP` only when ACPI RSDP is found
- `entry_pc`
- `entry_sp`
- `vm_enabled` (from `CR0.PG`)
- `vm_root_table` (from `CR3`)
- `uefi_system_table`
- `uefi_configuration_table`
- `memory_map`
- `memory_map_size`
- `memory_map_descriptor_size`
- `memory_map_descriptor_version`
- `acpi_rsdp` (searched from UEFI config table via ACPI 2.0/1.0 GUIDs)
- `framebuffer_*` (from GOP, when present)

Fields not currently populated are set to `0` and left clear in `valid_mask`.

## Non-UEFI Arch Stubs (Current)

`riscv64` currently provides:

- `abi_version`
- `arch_id`
- `valid_mask` includes:
  - `BOOT_INFO_HAS_ENTRY_STATE`
  - `BOOT_INFO_HAS_VM_STATE` when `satp != 0`
  - `BOOT_INFO_HAS_ARCH_DATA`
  - `BOOT_INFO_HAS_BOOT_CPU_ID`
  - `BOOT_INFO_HAS_DTB` when `dtb_ptr != 0`
  - `BOOT_INFO_HAS_SERIAL`
- `boot_cpu_id` from entry register `a0`
- `dtb_ptr` from entry register `a1`
- `serial_port = 0x10000000` (QEMU virt UART)
- extension data includes `satp`, UART base, and QEMU RAM profile

`mips` and `sparc64` currently provide:

- `abi_version`
- `arch_id`
- `valid_mask` includes:
  - `BOOT_INFO_HAS_ENTRY_STATE`
  - `BOOT_INFO_HAS_ARCH_DATA`
  - `BOOT_INFO_HAS_SERIAL`
- `serial_port` from architecture console backend base
- extension data includes captured firmware/entry register context and QEMU RAM profile

All other fields are currently zero until architecture-specific entry capture is added.

## Extension Structs (Current)

- UEFI path (`x86_64`/`arm64`): `boot_info_ext_uefi_t`
- `riscv64`: `boot_info_ext_riscv64_t`
- `mips`: `boot_info_ext_mips_t` (placeholder)
- `sparc64`: `boot_info_ext_sparc64_t` (placeholder)

## Rule

`kmain()` must treat `boot_info` as the only boot input contract.
Direct firmware/global assumptions outside `boot_info` are out of contract.
