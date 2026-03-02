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

## MMU Mapping API Backend

File: `arch/x86_64/mm/mmu_backend.c`

Implements architecture hooks used by generic `mm_map` API:
- `arch_mm_page_size` -> `2 MiB`
- `arch_mm_map_page`
- `arch_mm_unmap_page`
- `arch_mm_protect_page`
- `arch_mm_translate_page`
- `arch_mm_sync_tlb`

Flag mapping details:
- writable: PTE `RW`
- user: PTE `US`
- global: PTE `G`
- device: `PCD|PWT`
- non-exec: NX bit set

Current supported range:
- first 4 GiB identity window created by early paging

TLB maintenance:
- local CR3 reload for synchronization.
