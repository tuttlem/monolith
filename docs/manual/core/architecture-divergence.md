# Architecture Divergence Guide

This page highlights where execution paths diverge between `x86_64`, `arm64`, and `riscv64`.

## Boot Entry Model

- `x86_64`: UEFI `efi_main` entry, ACPI through UEFI config tables, boot extensions in `boot_info_ext_uefi_t`.
- `arm64`: UEFI `efi_main` entry, ACPI from UEFI config tables when present, boot extensions in `boot_info_ext_uefi_t`.
- `riscv64`: non-UEFI `arch_main(hart_id, dtb_ptr)` entry, DTB-driven handoff, boot extensions in `boot_info_ext_riscv64_t`.

## Console Backend

- `x86_64`/`arm64`: UEFI text output protocol in boot stage.
- `riscv64`: MMIO UART output in boot stage.

## CPU-Local Base Register

- `x86_64`: `MSR_GS_BASE`.
- `arm64`: `TPIDR_EL1`.
- `riscv64`: `tp` register.

## Barrier and TLB/I-cache Ops

- `x86_64`: `mfence/lfence/sfence`, CR3 self-write for local TLB sync.
- `arm64`: `DSB/DMB`, `TLBI` + barriers.
- `riscv64`: `fence` family and `sfence.vma`.

## Interrupt Controller Backend

- `x86_64`: legacy PIC backend currently active.
- `arm64`: GICv2 backend with claim/EOI flow.
- `riscv64`: controller shim/stub layer; full PLIC/AIA backend not yet active.

## Timer Backend

- `x86_64`: PIT periodic timer path.
- `arm64`: generic virtual timer (`CNTV_*`).
- `riscv64`: backend present but constrained by current interrupt-controller maturity.

## MMU Backend Granule/Shape

- `x86_64`: 2 MiB large-page style backend in current implementation.
- `arm64`: L2 block mapping granule in current implementation.
- `riscv64`: simplified large-granule Sv39 path in current implementation.

## PCI Enumeration Backend

- `x86_64`: implemented via legacy config I/O scanning.
- `arm64`: deferred in current phase.
- `riscv64`: deferred in current phase.

## USB Enumeration Backend

- All architectures currently use the same skeleton path.
- USB hosts are derived from enumerated PCI USB controller class codes.
- Full controller operation and class drivers are deferred.

## SMP Bootstrap Behavior

- `x86_64`: UEFI MP Services enumeration and AP startup.
- `arm64`: UEFI MP Services enumeration and AP startup.
- `riscv64`: possible CPU count from DTB, secondary start deferred.

## Discovery Sources

- `x86_64`: primarily ACPI, fallback population when unavailable.
- `arm64`: ACPI and/or DTB depending on firmware exposure.
- `riscv64`: DTB-driven discovery with fallback records.

## Panic/Exception Metadata

- Generic `interrupt_frame_t` and `exception_info_t` are common.
- Architecture-specific trap translation logic differs in backend interrupt files.

## Practical Guidance

- Write subsystem logic against common interfaces in `kernel/include`.
- Keep architecture conditionals inside backend files under `arch/<arch>/`.
- Use `boot_info_t.valid_mask` and normalized `hw_desc_t` to avoid firmware-format coupling.
