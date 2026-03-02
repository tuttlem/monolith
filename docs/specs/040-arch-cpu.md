# 040 Architecture CPU Layer (`arch_cpu`)

## Goal

Provide a stable CPU primitive API used by all generic subsystems.

## API Surface (Target)

```c
status_t arch_cpu_early_init(const boot_info_t *boot_info);
status_t arch_cpu_late_init(void);

BOOT_U64 arch_cpu_id(void);
BOOT_U64 arch_cpu_count_hint(void);

void arch_cpu_relax(void);
void arch_cpu_halt(void);
void arch_cpu_reboot(void);

BOOT_U64 arch_cycle_counter(void);

void arch_barrier_full(void);
void arch_barrier_read(void);
void arch_barrier_write(void);

void arch_tlb_sync_local(void);
void arch_icache_sync_range(BOOT_U64 addr, BOOT_U64 size);
```

## Per-Architecture Expectations

### x86_64
- cpu id from APIC id or fallback
- cycle counter via `rdtsc`/`rdtscp`
- barriers via `mfence/lfence/sfence` or compiler+serializing instructions
- halt/reboot path defined

### arm64
- cpu id via `MPIDR_EL1`
- counter via `CNTVCT_EL0` or equivalent
- barriers via `dsb`/`dmb`/`isb`
- icache sync hook functional

### riscv64
- hart id from CSR/boot metadata
- cycle counter via `cycle` CSR (or platform-safe alternative)
- barriers via `fence`/`fence.i`

## Contracts

- all functions callable without dynamic allocation
- `arch_cpu_early_init` must be idempotent or detect duplicate init
- no backend function should silently no-op unless documented

## Acceptance Criteria

- generic code no longer directly uses architecture inline asm for core CPU primitives
- cycle counter available on all architectures (or explicit status if unavailable)
- per-arch capability flags published

## Implementation Notes (Current Repository)

`040-arch-cpu` is implemented with real per-architecture backends:
- `arch/x86_64/cpu/cpu.c`
- `arch/arm64/cpu/cpu.c`
- `arch/riscv64/cpu/cpu.c`

`kernel/include/arch_cpu.h` now exposes the full frozen API (v1.0), and no
longer uses inline stub implementations.

Current behavior:
- `arch_cpu_early_init` validates architecture and captures boot CPU identity.
- `arch_cycle_counter` is implemented on all three architectures.
- barriers/TLB/i-cache hooks are implemented with architecture instructions.
- `kmain` idle loop now uses `arch_cpu_halt()`.

Note: `arch_cpu_count_hint()` currently returns `1` across architectures until
SMP discovery wiring in later specs.
