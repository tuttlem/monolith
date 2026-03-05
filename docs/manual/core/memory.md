# Memory Stack

This project's early memory stack has 4 layers:
1. architecture MMU handoff stabilization (`arch_memory_init`)
2. architecture-neutral MMU mapping API (`mm_map` family)
3. physical page allocator (`page_alloc`)
4. kernel heap allocator (`kmalloc`)

## 1) `arch_memory_init`

Prototype: `status_t arch_memory_init(boot_info_t *boot_info)`

Goal:
- Establish/refresh the architecture's own early page-table root.
- Update `boot_info.vm_enabled` and `boot_info.vm_root_table`.
- Populate extension diagnostics (`mem_init_status`, old/new root, mapped bytes).

Architecture implementations:
- x86_64: `arch/x86_64/mm/memory_init.c` + `early_paging.c`
- arm64: `arch/arm64/mm/memory_init.c` + `early_paging.c`
- riscv64: `arch/riscv64/mm/memory_init.c` + `early_paging.c`

## 2) MMU Mapping API (`mm_map`, `mm_unmap`, ...)

Public API: `kernel/include/arch_mm.h`  
Generic implementation: `kernel/mm/mmu.c`  
Per-arch backends:
- `arch/x86_64/mm/mmu_backend.c`
- `arch/arm64/mm/mmu_backend.c`
- `arch/riscv64/mm/mmu_backend.c`

### Types and flags

- `mm_virt_addr_t`, `mm_phys_addr_t`: 64-bit address types.
- `mmu_prot_t` flags:
  - `MMU_PROT_READ`
  - `MMU_PROT_WRITE`
  - `MMU_PROT_EXEC`
  - `MMU_PROT_USER`
  - `MMU_PROT_DEVICE`
  - `MMU_PROT_GLOBAL`

### Generic API functions

- `status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, u64 size, u64 prot_flags)`
- `status_t mm_unmap(mm_virt_addr_t va, u64 size)`
- `status_t mm_protect(mm_virt_addr_t va, u64 size, u64 prot_flags)`
- `status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags)`
- `status_t mm_sync_tlb(mm_virt_addr_t va, u64 size)`
- `u64 mm_page_size(void)`

### Alignment and multi-page semantics

- all map/unmap/protect operations require:
  - non-zero `size`
  - `va` aligned to backend page granule
  - `pa` aligned for map
  - `size` aligned to backend page granule
- operations iterate one backend page at a time.

Partial-failure policy:
- `mm_map`: if page `N` fails, pages `0..N-1` mapped in this call are rolled back.
- `mm_unmap`: stops on first failure; pages already unmapped remain unmapped.
- `mm_protect`: stops on first failure; pages already updated remain updated.

### Current backend mapping granularity

- x86_64: `2 MiB` (large pages inside 4 GiB identity window)
- arm64: `2 MiB` (L2 block entries inside 4 GiB identity window)
- riscv64: `1 GiB` (Sv39 root leaf entries inside 4 GiB identity window)

Current limitation: backends intentionally operate within the early identity-mapped 4 GiB window only.

## 3) Page Allocator (`page_alloc`)

API: `kernel/include/page_alloc.h`
Implementation: `kernel/mm/page_alloc.c`

### Data model

- Uses normalized `boot_info.memory_regions[]`.
- Filters `kind == BOOT_MEM_REGION_USABLE`.
- Converts usable regions into page ranges.
- Allocates linearly (bump) then reuses freed pages via recycle stack.

### Important constants

- page size: 4096 bytes
- usable-range cap: `MAX_USABLE_RANGES` (64)
- recycle stack cap: `MAX_RECYCLED_PAGES` (256)
- allocation floor:
  - x86_64 / arm64: `1 MiB`
  - riscv64: `0x80400000`

### Functions

- `status_t page_alloc_init(boot_info_t *boot_info)`
  - initializes internal ranges and counters
  - returns `STATUS_OK` when pages are available
- `u64 alloc_page(void)`
  - returns physical address of 4KiB page, or `0` on exhaustion/unavailable
- `void free_page(u64 phys_addr)`
  - validates alignment and pushes into recycle stack if capacity allows
- `void page_alloc_stats(page_alloc_stats_t *out)`
  - reports availability and page counts

## 4) Heap Allocator (`kmalloc`)

API: `kernel/include/kmalloc.h`
Implementation: `kernel/mm/kmalloc.c`

### Design

- Page-backed allocator.
- Each allocated physical page is treated as allocator metadata + blocks.
- Block headers support splitting and coalescing.
- Allocations are 16-byte aligned.

### Internal structures

- `kmalloc_page_t`
  - page metadata (`phys_addr`, `free_bytes`, linked-list links)
  - `first` block pointer
- `kmalloc_block_t`
  - block metadata (`size`, `free`, prev/next links)

### Behavior

- `kmalloc(size)` scans pages for free block, splits when useful.
- If no block fits, obtains a new page from `alloc_page()`.
- `kfree(ptr)` validates pointer, marks block free, coalesces neighbors.
- Completely free pages are returned to page allocator with `free_page()`.

### Stats and validation

- `kmalloc_stats(kmalloc_stats_t *out)` gives usage and failure counters.
- counters include invalid/double-free tracking.
- `kmalloc_self_test()` exercises allocation/free reuse patterns.

## Usage Order Contract

Call in this order:
1. `arch_memory_init` / `arch_mm_early_init`
2. `page_alloc_init`
3. `kmalloc_init`

Call the MMU mapping API (`mm_map` family) only after early MMU handoff (`arch_mm_early_init`) succeeds.

Do not call `kmalloc` before `kmalloc_init` succeeds.
