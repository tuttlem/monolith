# SMP Bootstrap and IPI Substrate

API: `kernel/include/smp.h`  
Implementation: `kernel/smp.c`

## Scope

Current phase provides generic SMP bootstrap plus baseline IPI/TLB shootdown contracts.

Implemented APIs:
- `status_t smp_init(const boot_info_t *boot_info)`
- `status_t smp_cpu_start(u64 cpu_id)`
- `u64 smp_cpu_count_online(void)`
- `u64 smp_cpu_count_possible(void)`
- `void smp_secondary_entry(u64 cpu_id)`
- `status_t ipi_send(u64 cpu_id, ipi_kind_t kind)`
- `status_t tlb_shootdown(cpu_mask_t mask, virt_addr_t va, u64 len)`

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
- `status_t arch_smp_bootstrap(const boot_info_t *, u64 *out_possible, u64 *out_started)`
- `status_t arch_smp_cpu_start(u64 cpu_id)`
- `status_t arch_smp_ipi_send(u64 cpu_id, u64 kind)`
- `status_t arch_smp_tlb_shootdown(u64 mask, u64 va, u64 len)`

Current backend files:
- `arch/x86_64/cpu/smp.c`
- `arch/arm64/cpu/smp.c`
- `arch/riscv64/cpu/smp.c`

Current backend status:
- x86_64: uses UEFI MP Services protocol when available; attempts AP callback startup path.
- arm64: uses UEFI MP Services protocol when available; attempts AP callback startup path.
- riscv64: parses DTB CPU topology for `possible` count and reports active baseline status with explicit unsupported returns for unimplemented remote start/IPI paths.
- all architectures: local-CPU IPI and local TLB sync paths return `STATUS_OK`; remote operations return explicit `STATUS_NOT_SUPPORTED` until full controller/HSM startup flow is added.

Status semantics:
- `STATUS_OK`: backend performed startup flow (or single-CPU platform)
- `STATUS_NOT_SUPPORTED`: operation is defined but remote backend mechanism is not available on current platform path
- `STATUS_TRY_AGAIN`: startup mechanism present but bootstrap attempt failed
