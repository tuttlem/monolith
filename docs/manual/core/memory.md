# Memory Stack

This project's early memory stack has 3 layers:
1. architecture MMU handoff stabilization (`arch_memory_init`)
2. physical page allocator (`page_alloc`)
3. kernel heap allocator (`kmalloc`)

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

## 2) Page Allocator (`page_alloc`)

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
- `BOOT_U64 alloc_page(void)`
  - returns physical address of 4KiB page, or `0` on exhaustion/unavailable
- `void free_page(BOOT_U64 phys_addr)`
  - validates alignment and pushes into recycle stack if capacity allows
- `void page_alloc_stats(page_alloc_stats_t *out)`
  - reports availability and page counts

## 3) Heap Allocator (`kmalloc`)

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
1. `arch_memory_init`
2. `page_alloc_init`
3. `kmalloc_init`

Do not call `kmalloc` before `kmalloc_init` succeeds.
