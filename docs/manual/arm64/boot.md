# arm64 Boot

## Producer

`efi_main` in `arch/arm64/boot/efi_main.c` constructs `boot_info_t` and `boot_info_ext_uefi_t`, then calls `kmain`.

## Current Entry-State Coverage

- sets `arch_id = BOOT_INFO_ARCH_ARM64`
- sets UEFI pointers and captures memory map/regions
- captures framebuffer metadata when GOP is present
- currently leaves `entry_pc`, `entry_sp`, and `vm_root_table` as zero in this UEFI path

That is acceptable for now because runtime bring-up does not yet consume those fields on arm64, but it is a future hardening target.

## UEFI Memory and Framebuffer

Same normalized memory-region model as x86_64:
- converts UEFI descriptor types into `BOOT_MEM_REGION_*`
- populates `memory_regions[]` for core allocators
- sets `BOOT_INFO_HAS_MEM_REGIONS` when populated

## Extension Payload

`boot_info_ext_uefi_t` is attached through `arch_data_ptr` and later updated by memory init diagnostics.
