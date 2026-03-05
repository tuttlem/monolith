# 100 SMP Bootstrap Skeleton

## Goal

Bring up secondary CPUs into a shared C entry and park them safely.

## Phase Scope

- discover possible CPUs
- start secondary CPUs
- run a common secondary entry routine
- publish online state
- park in idle loop (no scheduler yet)

## Generic APIs

- `status_t smp_init(const boot_info_t *boot_info)`
- `u64 smp_cpu_count_online(void)`
- `u64 smp_cpu_count_possible(void)`

## Architecture Responsibilities

### x86_64
- AP startup protocol (SIPI path)
- LAPIC dependency ordering

### arm64
- PSCI CPU_ON (if available) or platform bring-up path

### riscv64
- SBI HSM extension path or platform firmware handoff mechanism

## Synchronization Model

- boot CPU owns startup sequence
- secondaries publish `online=1` in per-CPU state
- bounded wait with timeout and status reporting

## Acceptance Criteria

- at least one secondary CPU can reach common entry on supported machine models
- failure is reported via `status_t` and diagnostic logs, not silent hang
