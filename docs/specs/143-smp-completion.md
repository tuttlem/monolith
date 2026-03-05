# 143 SMP Completion

## Goal

Complete SMP substrate beyond bootstrap parking so multicore control is practical for any downstream OS design.

## In Scope

- Secondary CPU startup to common C entry for all architectures.
- CPU online/offline state machine.
- Inter-processor interrupt (IPI) API:
  - reschedule
  - tlb shootdown
  - function call.
- TLB shootdown core hooks.
- Per-CPU timer integration.

## Out of Scope

- Scheduler load balancing policy.
- CPU hotplug policy model.

## Public Interfaces

- Headers: `kernel/include/smp.h`, `kernel/include/ipi.h`
- APIs:
  - `status_t smp_init(const boot_info_t *boot_info)`
  - `status_t smp_cpu_start(u64 cpu_id)`
  - `status_t ipi_send(u64 cpu_id, ipi_kind_t kind)`
  - `status_t tlb_shootdown(cpu_mask_t mask, virt_addr_t va, u64 len)`

## Architecture Backends

- `x86_64`: LAPIC startup/IPI flow.
- `arm64`: PSCI `CPU_ON` + GIC SGIs.
- `riscv64`: SBI HSM + software interrupt/IPI path.

## Tests

- `-smp N` boot test verifies `possible` and `online` counts.
- IPI ping test between cores.
- simple shootdown test validates no stale translation use.

## Acceptance Criteria

1. Multicore bring-up works on platforms where firmware/virt machine expose SMP.
2. Generic code uses IPI/TLB APIs, not controller-specific paths.
3. Single-core fallback remains clean and explicit.
