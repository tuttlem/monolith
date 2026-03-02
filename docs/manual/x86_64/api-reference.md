# x86_64 API Reference

This page documents x86_64-specific public symbols and how x86_64 diverges from arm64/riscv64 behavior.

## Boot and Console

### `EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)`
- File: `arch/x86_64/boot/efi_main.c`
- Purpose: UEFI entry point that builds `boot_info_t` and enters `kmain`.
- Parameters:
  - `image_handle`: EFI image handle for this binary.
  - `system_table`: EFI system table pointer.
- Returns: EFI status code (`EFI_SUCCESS` on normal handoff path).
- Divergence:
  - Uses x86 control register state (`CR0/CR3`) and captures RIP/RSP snapshots.

### `void arch_puts(const char *s)` / `void arch_halt(void)` / `void arch_panic_stop(void)`
- Purpose: x86 early console and terminal halt behavior.
- Notes:
  - `arch_puts` writes through UEFI text output in boot phase.
  - `arch_halt` executes `hlt` in a loop.

## CPU Layer (`arch_cpu.h` backend)

Implemented in `arch/x86_64/cpu/cpu.c`:
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
- CPU-local base uses `MSR_GS_BASE`.
- `arch_cpu_relax` uses `pause`.
- `arch_cycle_counter` uses `rdtsc`.

## SMP Backend

### `status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus)`
- File: `arch/x86_64/cpu/smp.c`
- Purpose: SMP discovery/bring-up through UEFI MP Services.
- Parameters:
  - `boot_info`: must include valid UEFI extension data.
  - `out_possible_cpus`: receives total processors reported by firmware.
  - `out_started_cpus`: receives number of APs started.
- Returns:
  - `STATUS_OK` if path runs successfully.
  - `STATUS_DEFERRED` if MP Services unavailable.
  - error status on hard failure.
- Example:
```c
BOOT_U64 possible = 1, started = 0;
status_t st = arch_smp_bootstrap(info, &possible, &started);
```

## Memory Bring-up

### `status_t arch_memory_init(boot_info_t *boot_info)`
- File: `arch/x86_64/mm/memory_init.c`
- Purpose: architecture memory takeover wrapper and boot extension updates.

### `int x86_64_early_paging_takeover(x86_64_early_paging_result_t *result)`
- File: `arch/x86_64/mm/early_paging.c`
- Purpose: build/load new paging root and preserve identity mapping for early kernel execution.
- Parameters:
  - `result`: output structure for old/new roots and mapped byte count.
- Returns: `0` on success, non-zero on failure.

### `x86_64_early_paging_result_t`
- Members:
  - `old_cr3`: CR3 before takeover.
  - `new_cr3`: CR3 after takeover.
  - `identity_bytes_mapped`: contiguous identity span kept valid.

## MMU Backend (`arch_mm.h` backend)

Implemented in `arch/x86_64/mm/mmu_backend.c`:
- `arch_mm_page_size`
- `arch_mm_map_page`
- `arch_mm_unmap_page`
- `arch_mm_protect_page`
- `arch_mm_translate_page`
- `arch_mm_sync_tlb`

Divergence notes:
- Uses 2 MiB large-page granule in current backend.
- Mapping range is constrained by current top-level static table window.

## Interrupt Backend

### `status_t arch_interrupts_init(const boot_info_t *boot_info)`
### `void arch_interrupts_enable(void)`
### `void arch_interrupts_disable(void)`
### `void arch_exception_selftest_trigger(void)`
- File: `arch/x86_64/irq/interrupts.c`
- Purpose: IDT setup, mask control via `sti/cli`, and test exception generation.

### `status_t x86_64_pic_controller_init(const boot_info_t *boot_info)`
- File: `arch/x86_64/irq/pic.c`
- Purpose: initialize legacy PIC controller and register IRQ controller ops.
- Divergence:
  - Current x86 IRQ controller path is PIC-based, not IOAPIC/LAPIC yet.

## Timer Backend

### `status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector)`
### `void arch_timer_ack(BOOT_U64 vector)`
### `BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info)`
- File: `arch/x86_64/timer/timer.c`
- Purpose: PIT periodic timer setup and IRQ ack path.
- Divergence:
  - Periodic source is PIT IRQ0 path.
