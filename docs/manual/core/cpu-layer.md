# CPU Layer (`arch_cpu`, Spec 040)

API: `kernel/include/arch_cpu.h`

## Core Functions

Initialization:
- `arch_cpu_early_init(const boot_info_t *boot_info)`
- `arch_cpu_late_init(void)`

Identity and topology hints:
- `arch_cpu_id(void)`
- `arch_cpu_count_hint(void)`

Execution controls:
- `arch_cpu_relax(void)`
- `arch_cpu_halt(void)`
- `arch_cpu_reboot(void)`

Counters and memory ordering:
- `arch_cycle_counter(void)`
- `arch_barrier_full(void)`
- `arch_barrier_read(void)`
- `arch_barrier_write(void)`
- `arch_tlb_sync_local(void)`
- `arch_icache_sync_range(u64 addr, u64 size)`

CPU-local base hooks (used by per-CPU runtime):
- `arch_cpu_set_local_base(u64 base)`
- `arch_cpu_get_local_base(void)`

## Backend Mapping

- x86_64: `arch/x86_64/cpu/cpu.c`
- arm64: `arch/arm64/cpu/cpu.c`
- riscv64: `arch/riscv64/cpu/cpu.c`

All backends provide real implementations for current bring-up.
