# x86_64 Boot

## Producer

`efi_main` in `arch/x86_64/boot/efi_main.c` constructs `boot_info_t` and `boot_info_ext_uefi_t`, then calls `kmain(&boot_info)`.

## Captured CPU State

- `entry_pc` from `RIP`
- `entry_sp` from `RSP`
- `vm_enabled` from `CR0.PG`
- `vm_root_table` from `CR3`

## UEFI Data Captured

- system/configuration table pointers
- full UEFI memory descriptor stream
- normalized memory regions (`memory_regions[]`)
- ACPI RSDP (ACPI 2.0 or 1.0 GUID search)
- GOP framebuffer metadata (if protocol present)

## Extension Payload

`boot_info_ext_uefi_t` includes:
- image/system/runtime/boot services pointers
- console handles
- firmware vendor/revision
- memory-init/paging diagnostics updated later by `arch_memory_init`

## Console Behavior

`arch_puts` writes through UEFI text output and translates `\n` to `\r\n`.
