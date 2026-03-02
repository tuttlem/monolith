# arm64 API Reference

This page documents arm64-specific public symbols and arm64-specific execution behavior.

## Boot and Console

### `EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)`
- File: `arch/arm64/boot/efi_main.c`
- Purpose: UEFI entry point that builds `boot_info_t` and enters `kmain`.
- Parameters:
  - `image_handle`: EFI image handle.
  - `system_table`: EFI system table pointer.
- Returns: EFI status code.
- Divergence:
  - Resolves ACPI RSDP through UEFI config tables when present.
  - Populates arm64 VM state (`TTBR0`-style root via extension fields).

### `void arch_puts(const char *s)` / `void arch_halt(void)` / `void arch_panic_stop(void)`
- Purpose: arm64 early console and halt behavior.

## CPU Layer (`arch_cpu.h` backend)

Implemented in `arch/arm64/cpu/cpu.c`:
- `arch_cpu_early_init`
- `arch_cpu_late_init`
- `arch_cpu_id`
- `arch_cpu_count_hint`
- `arch_cpu_relax`
- `arch_cpu_halt`
- `arch_cpu_reboot`
- `arch_cycle_counter`
- `arch_cpu_set_local_base`
- `arch_cpu_get_local_base`
- `arch_barrier_full`
- `arch_barrier_read`
- `arch_barrier_write`
- `arch_tlb_sync_local`
- `arch_icache_sync_range`

Divergence notes:
- CPU-local base uses `TPIDR_EL1`.
- Memory barriers use ARM `DSB/DMB` forms.
- `arch_cycle_counter` uses virtual counter register (`CNTVCT_EL0`).

## SMP Backend

### `status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus)`
- File: `arch/arm64/cpu/smp.c`
- Purpose: use UEFI MP Services for CPU enumeration/AP startup.
- Parameters:
  - `boot_info`: requires valid UEFI extension data.
  - `out_possible_cpus`: firmware-reported processors.
  - `out_started_cpus`: APs that reached secondary entry.
- Returns: status as defined by `arch_smp.h` contract.

## Memory Bring-up

### `status_t arch_memory_init(boot_info_t *boot_info)`
- File: `arch/arm64/mm/memory_init.c`
- Purpose: arm64 early memory takeover wrapper and extension result fill.

### `int arm64_early_paging_takeover(const boot_info_t *boot_info, arm64_early_paging_result_t *result)`
- File: `arch/arm64/mm/early_paging.c`
- Purpose: installs early translation tables and MAIR setup.
- Parameters:
  - `boot_info`: used for memory-kind-aware attribute assignment.
  - `result`: output old/new roots and mapped span.
- Returns: `0` success, non-zero failure.

### `arm64_early_paging_result_t`
- Members:
  - `old_ttbr0`: TTBR0 before takeover.
  - `new_ttbr0`: TTBR0 after takeover.
  - `identity_bytes_mapped`: identity span preserved.

## MMU Backend (`arch_mm.h` backend)

Implemented in `arch/arm64/mm/mmu_backend.c`:
- `arch_mm_page_size`
- `arch_mm_map_page`
- `arch_mm_unmap_page`
- `arch_mm_protect_page`
- `arch_mm_translate_page`
- `arch_mm_sync_tlb`

Divergence notes:
- Uses L2 block mapping granule (2 MiB equivalent) in current implementation.
- Attribute encoding maps generic `MMU_PROT_*` onto ARM block attributes.

## Interrupt Backend

### `status_t arch_interrupts_init(const boot_info_t *boot_info)`
### `void arch_interrupts_enable(void)`
### `void arch_interrupts_disable(void)`
### `void arch_exception_selftest_trigger(void)`
- File: `arch/arm64/irq/interrupts.c`
- Purpose: VBAR setup, DAIF mask control, and exception self-test.

### GICv2 public helpers (`arch/arm64/irq/gicv2.c`)
- `status_t arm64_gicv2_controller_init(const boot_info_t *boot_info)`
- `status_t arm64_gicv2_claim_irq(BOOT_U64 *out_irq)`
- `void arm64_gicv2_eoi_irq(BOOT_U64 irq)`

Function details:
- `arm64_gicv2_controller_init`:
  - Purpose: bind GICv2 distributor/CPU interface and register generic IRQ controller ops.
  - Params: `boot_info` with discovered GIC MMIO data.
  - Returns: status.
- `arm64_gicv2_claim_irq`:
  - Purpose: read currently active IRQ ID from CPU interface.
  - Params: `out_irq` output pointer.
  - Returns: status.
- `arm64_gicv2_eoi_irq`:
  - Purpose: signal end of interrupt to CPU interface.

## Timer Backend

### `status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector)`
### `void arch_timer_ack(BOOT_U64 vector)`
### `BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info)`
- File: `arch/arm64/timer/timer.c`
- Purpose: arm generic virtual timer setup and acknowledgment path.
- Divergence:
  - Uses architected timer registers (`CNTFRQ`, `CNTV_TVAL`, `CNTV_CTL`).
