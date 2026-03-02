# riscv64 Memory

## `arch_memory_init`

File: `arch/riscv64/mm/memory_init.c`

Behavior:
1. validates architecture id and required extension payload.
2. computes mapped-bytes metric from usable normalized regions.
3. runs `riscv64_early_paging_takeover`.
4. updates:
   - `boot_info.vm_enabled = 1`
   - `boot_info.vm_root_table = new_satp`
   - riscv extension memory-init diagnostics.

## Early Paging Takeover

File: `arch/riscv64/mm/early_paging.c`

Current mapping model:
- allocates static aligned root page table (`g_root[512]`).
- builds 4 GiB identity map with leaf entries (RWX + A/D bits).
- preserves existing mode from old `satp`; if bare mode, defaults to Sv39.
- writes new `satp` and executes `sfence.vma`.

Result metadata:
- old/new `satp`
- identity bytes mapped = 4 GiB

## Practical Implication

RISC-V gets stable early VM handoff and can use the same generic page allocator + kmalloc layers as other active architectures.

## MMU Mapping API Backend

File: `arch/riscv64/mm/mmu_backend.c`

Implements architecture hooks used by generic `mm_map` API:
- `arch_mm_page_size` -> `1 GiB`
- `arch_mm_map_page`
- `arch_mm_unmap_page`
- `arch_mm_protect_page`
- `arch_mm_translate_page`
- `arch_mm_sync_tlb`

Flag mapping details:
- read/write/exec/user/global map to Sv39 PTE `R/W/X/U/G`
- accessed/dirty management sets `A`, and `D` when writable

Current supported range:
- first 4 GiB identity window from early paging root entries

TLB maintenance:
- `sfence.vma x0, x0`
