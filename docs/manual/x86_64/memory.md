# x86_64 Memory

## `arch_memory_init`

File: `arch/x86_64/mm/memory_init.c`

Behavior:
1. validates `boot_info` and architecture id.
2. initializes UEFI extension memory-init status fields.
3. calls `x86_64_early_paging_takeover`.
4. on success, updates:
   - `boot_info.vm_enabled = 1`
   - `boot_info.vm_root_table = new_cr3`
   - extension diagnostics (`old/new root`, mapped bytes).

## Early Paging Takeover

File: `arch/x86_64/mm/early_paging.c`

Current mapping model:
- fresh page tables in static aligned arrays:
  - `PML4[512]`
  - `PDPT[512]`
  - `PD[4][512]`
- identity map first 4 GiB using 2 MiB large pages.
- write new `CR3`.

Mapped bytes reported: `4 GiB`.

## Practical Implication

After `arch_memory_init`, kernel runs on its own known page table root and page allocator can consume normalized usable regions.
