# Boot Info ABI

This document defines the kernel boot input ABI frozen in `BOOT_INFO_ABI_VERSION = 1`.

All kernel entry paths now pass exactly one input:

```c
void kmain(const boot_info_t *boot_info);
```

The struct is defined in `kernel/include/boot_info.h`.

## Field Contract

- `abi_version`: ABI version. Must be `BOOT_INFO_ABI_VERSION`.
- `entry_rip`: Instruction pointer snapshot taken in architecture entry code.
- `entry_rsp`: Stack pointer snapshot taken in architecture entry code.
- `paging_enabled`: `1` if paging is enabled at boot entry, else `0`.
- `current_page_map`: Active top-level page map base (`CR3` on x86_64).
- `uefi_system_table`: Raw pointer to `EFI_SYSTEM_TABLE` when booted via UEFI.
- `uefi_configuration_table`: Raw pointer to UEFI configuration table array.
- `memory_map`: Pointer to firmware memory map descriptor stream.
- `memory_map_size`: Total bytes at `memory_map`.
- `memory_map_descriptor_size`: Size of one memory descriptor.
- `memory_map_descriptor_version`: Descriptor format version.
- `acpi_rsdp`: Pointer to ACPI RSDP if present, else `0`.
- `framebuffer_base`: Framebuffer base address if discovered, else `0`.
- `framebuffer_width`: Framebuffer width in pixels.
- `framebuffer_height`: Framebuffer height in pixels.
- `framebuffer_pixels_per_scanline`: Framebuffer pitch in pixels.
- `framebuffer_format`: Platform-defined pixel format enum value.
- `serial_port`: Serial base or handle if known, else `0`.

## x86_64 UEFI Population (Current)

In `arch/x86_64/boot/efi_main.c`, these fields are actively populated:

- `abi_version`
- `entry_rip`
- `entry_rsp`
- `paging_enabled` (from `CR0.PG`)
- `current_page_map` (from `CR3`)
- `uefi_system_table`
- `uefi_configuration_table`
- `acpi_rsdp` (searched from UEFI configuration table via ACPI 2.0/1.0 GUIDs)

These are currently reserved but not populated yet (set to `0`):

- `memory_map*`
- `framebuffer*`
- `serial_port`

## Rule

`kmain()` must treat `boot_info` as the only boot input contract.
Direct firmware/global assumptions outside `boot_info` are out of contract.
