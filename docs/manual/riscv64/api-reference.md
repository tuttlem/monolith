# riscv64 API Reference

This page documents riscv64-specific public symbols and riscv64-specific execution behavior.

## Boot and Console

### `void arch_main(BOOT_U64 hart_id, BOOT_U64 dtb_ptr)`
- File: `arch/riscv64/boot/main.c`
- Purpose: non-UEFI machine entry path that builds `boot_info_t` and enters `kmain`.
- Parameters:
  - `hart_id`: boot hart ID (`a0`).
  - `dtb_ptr`: DTB pointer (`a1`), with fallback resolution for QEMU layout.
- Returns: none.
- Divergence:
  - Uses DTB parsing for RAM and discovery hints.
  - Populates `boot_info_ext_riscv64_t` instead of UEFI extension.

### `void arch_puts(const char *s)` / `void arch_halt(void)` / `void arch_panic_stop(void)`
- File: `arch/riscv64/boot/console.c`
- Purpose: UART-backed early console and halt implementation.

## CPU Layer (`arch_cpu.h` backend)

Implemented in `arch/riscv64/cpu/cpu.c`:
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
- CPU-local base uses `tp` register.
- Barriers use RISC-V `fence` instructions.
- Local TLB sync uses `sfence.vma`.

## SMP Backend

### `status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus)`
- File: `arch/riscv64/cpu/smp.c`
- Purpose: derive possible CPU count from DTB CPU nodes; AP startup is deferred.
- Parameters:
  - `boot_info`: must identify riscv64 handoff.
  - `out_possible_cpus`: receives discovered possible CPU count.
  - `out_started_cpus`: receives started AP count (currently `0`).
- Returns: `STATUS_DEFERRED` currently, by design.
- Remarks:
  - This path reports possible CPUs but does not yet launch secondary harts.

## Memory Bring-up

### `status_t arch_memory_init(boot_info_t *boot_info)`
- File: `arch/riscv64/mm/memory_init.c`
- Purpose: memory initialization wrapper and extension updates.

### `int riscv64_early_paging_takeover(riscv64_early_paging_result_t *result)`
- File: `arch/riscv64/mm/early_paging.c`
- Purpose: installs early SATP-based translation root.
- Parameters:
  - `result`: output old/new SATP and mapped bytes.
- Returns: `0` success, non-zero failure.

### `riscv64_early_paging_result_t`
- Members:
  - `old_satp`: SATP before takeover.
  - `new_satp`: SATP after takeover.
  - `identity_bytes_mapped`: identity-mapped span.

## MMU Backend (`arch_mm.h` backend)

Implemented in `arch/riscv64/mm/mmu_backend.c`:
- `arch_mm_page_size`
- `arch_mm_map_page`
- `arch_mm_unmap_page`
- `arch_mm_protect_page`
- `arch_mm_translate_page`
- `arch_mm_sync_tlb`

Divergence notes:
- Current backend uses large granule mappings in a simplified Sv39 layout.
- TLB synchronization is global/local `sfence.vma` style.

## Interrupt Backend

### `status_t arch_interrupts_init(const boot_info_t *boot_info)`
### `void arch_interrupts_enable(void)`
### `void arch_interrupts_disable(void)`
### `void arch_exception_selftest_trigger(void)`
- File: `arch/riscv64/irq/interrupts.c`
- Purpose: trap-vector setup and S-mode interrupt mask control.

### `status_t riscv64_irq_controller_init(const boot_info_t *boot_info)`
- File: `arch/riscv64/irq/controller_stub.c`
- Purpose: register current riscv64 IRQ controller shim.
- Parameters:
  - `boot_info`: handoff pointer.
- Returns: status.
- Remarks:
  - Current implementation is a controller stub contract layer; full PLIC/AIA backend is pending.

## Timer Backend

### `status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector)`
### `void arch_timer_ack(BOOT_U64 vector)`
### `BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info)`
- File: `arch/riscv64/timer/timer.c`
- Purpose: riscv64 timer backend wiring for generic timebase.
- Divergence:
  - Timer IRQ/controller interaction remains minimal while controller backend is stubbed.

## Syscall Trap Hook

### `status_t arch_syscall_init(const boot_info_t *boot_info)`
- File: `arch/riscv64/syscall/syscall.c`
- Purpose: riscv64 syscall trap-entry backend hook for transport ABI.
- Current phase behavior:
  - returns `STATUS_DEFERRED` while dedicated trap-entry plumbing is staged.
