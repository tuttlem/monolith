# SMP Bootstrap Skeleton (Spec 100)

API: `kernel/include/smp.h`  
Implementation: `kernel/smp.c`

## Scope

Current phase provides the generic SMP bootstrap contract and a common secondary C entry routine.

Implemented APIs:
- `status_t smp_init(const boot_info_t *boot_info)`
- `BOOT_U64 smp_cpu_count_online(void)`
- `BOOT_U64 smp_cpu_count_possible(void)`
- `void smp_secondary_entry(BOOT_U64 cpu_id)`

## Generic Flow

1. determine possible CPU count from `arch_cpu_count_hint()`
2. read current online CPUs from per-CPU table
3. call architecture bootstrap hook:
   - `arch_smp_bootstrap(boot_info, &possible, &started)`
4. if backends report started secondaries, wait in bounded loop for online publication
5. expose counters and return status

Timeout policy:
- bounded wait loop (`SMP_WAIT_LOOPS`)
- returns `STATUS_TRY_AGAIN` if expected secondaries do not publish online in time

## Secondary Entry Policy

`smp_secondary_entry(cpu_id)`:
1. registers current CPU in per-CPU table (`percpu_register_current_cpu`)
2. parks CPU in idle halt loop (no scheduler yet)

## Architecture Hook

Header: `kernel/include/arch_smp.h`  
Hook:
- `status_t arch_smp_bootstrap(const boot_info_t *, BOOT_U64 *out_possible, BOOT_U64 *out_started)`

Current backend files:
- `arch/x86_64/cpu/smp.c`
- `arch/arm64/cpu/smp.c`
- `arch/riscv64/cpu/smp.c`

Current backend status:
- returns `STATUS_DEFERRED` with `possible=1`, `started=0`
- this preserves cross-architecture stability while real AP/PSCI/SBI startup work is added next.
