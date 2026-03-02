# arm64 Memory

## `arch_memory_init`

File: `arch/arm64/mm/memory_init.c`

Behavior:
1. validates `boot_info` and architecture id.
2. requires `arch_data_ptr` with `boot_info_ext_uefi_t` (returns `STATUS_INTERNAL` if missing).
3. computes total usable bytes from normalized memory regions.
4. runs `arm64_early_paging_takeover`.
5. updates VM state and extension diagnostics.

## Early Paging Takeover

File: `arch/arm64/mm/early_paging.c`

Current mapping model:
- builds static aligned translation tables (`L0`, `L1`, `L2`).
- identity-maps first 4 GiB with L2 block entries (2 MiB granularity).
- chooses memory attributes from region kind:
  - normal WB for RAM-like regions
  - device attributes for MMIO regions
- writes `MAIR_EL1`.
- switches `TTBR0_EL1` to new table base and flushes TLB.

Result metadata:
- old/new TTBR0 base recorded
- identity bytes mapped = 4 GiB

## Practical Implication

After memory init, arm64 executes under kernel-installed table root with explicit attribute policy for MMIO vs normal memory.
