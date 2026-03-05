#include "page_alloc.h"

#define PAGE_SIZE 4096ULL
#define ONE_MIB 0x100000ULL
#define RISCV64_ALLOC_FLOOR 0x80400000ULL
#define MAX_USABLE_RANGES 64U
#define MAX_RECYCLED_PAGES 256U

typedef struct {
  u64 base;
  u64 page_count;
} page_range_t;

static struct {
  u64 initialized;
  u64 available;
  u64 total_pages;
  u64 free_pages;
  u32 range_count;
  u32 current_range;
  u64 current_offset_pages;
  u32 recycled_count;
  u64 recycled[MAX_RECYCLED_PAGES];
  page_range_t ranges[MAX_USABLE_RANGES];
} g_page_alloc;

static u64 align_up(u64 value, u64 align) {
  return (value + (align - 1ULL)) & ~(align - 1ULL);
}

static u64 align_down(u64 value, u64 align) {
  return value & ~(align - 1ULL);
}

static void clear_state(void) {
  u32 i;
  g_page_alloc.initialized = 1;
  g_page_alloc.available = 0;
  g_page_alloc.total_pages = 0;
  g_page_alloc.free_pages = 0;
  g_page_alloc.range_count = 0;
  g_page_alloc.current_range = 0;
  g_page_alloc.current_offset_pages = 0;
  g_page_alloc.recycled_count = 0;
  for (i = 0; i < MAX_USABLE_RANGES; ++i) {
    g_page_alloc.ranges[i].base = 0;
    g_page_alloc.ranges[i].page_count = 0;
  }
  for (i = 0; i < MAX_RECYCLED_PAGES; ++i) {
    g_page_alloc.recycled[i] = 0;
  }
}

static void add_usable_range(u64 base, u64 size, u64 floor, u64 bias) {
  u64 start;
  u64 end;
  u64 page_count;
  u32 idx;

  if (g_page_alloc.range_count >= MAX_USABLE_RANGES || size < PAGE_SIZE) {
    return;
  }

  start = align_up(base, PAGE_SIZE);
  end = align_down(base + size, PAGE_SIZE);
  if (end <= start) {
    return;
  }

  if (end <= floor) {
    return;
  }
  if (start < floor) {
    start = floor;
  }
  if (end <= start) {
    return;
  }

  page_count = (end - start) / PAGE_SIZE;
  if (page_count == 0) {
    return;
  }

  idx = g_page_alloc.range_count++;
  g_page_alloc.ranges[idx].base = start + bias;
  g_page_alloc.ranges[idx].page_count = page_count;
  g_page_alloc.total_pages += page_count;
  g_page_alloc.free_pages += page_count;
}

status_t page_alloc_init(boot_info_t *boot_info) {
  u32 i;
  u64 floor = ONE_MIB;
  u64 bias = 0;

  clear_state();

  if (boot_info == (boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  if (boot_info->arch_id != BOOT_INFO_ARCH_X86_64 && boot_info->arch_id != BOOT_INFO_ARCH_RISCV64 &&
      boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_NOT_SUPPORTED;
  }
  if (boot_info->arch_id == BOOT_INFO_ARCH_RISCV64) {
    floor = RISCV64_ALLOC_FLOOR;
  }

  for (i = 0; i < boot_info->memory_region_count; ++i) {
    const boot_mem_region_t *region = &boot_info->memory_regions[i];
    if (region->kind != BOOT_MEM_REGION_USABLE) {
      continue;
    }
    add_usable_range(region->base, region->size, floor, bias);
  }

  if (g_page_alloc.total_pages > 0) {
    g_page_alloc.available = 1;
    return STATUS_OK;
  }
  return STATUS_NO_MEMORY;
}

u64 alloc_page(void) {
  page_range_t *range;
  u64 addr;

  if (g_page_alloc.initialized == 0 || g_page_alloc.available == 0) {
    return 0;
  }

  if (g_page_alloc.recycled_count > 0) {
    --g_page_alloc.recycled_count;
    --g_page_alloc.free_pages;
    return g_page_alloc.recycled[g_page_alloc.recycled_count];
  }

  while (g_page_alloc.current_range < g_page_alloc.range_count) {
    range = &g_page_alloc.ranges[g_page_alloc.current_range];
    if (g_page_alloc.current_offset_pages < range->page_count) {
      addr = range->base + (g_page_alloc.current_offset_pages * PAGE_SIZE);
      ++g_page_alloc.current_offset_pages;
      --g_page_alloc.free_pages;
      return addr;
    }
    ++g_page_alloc.current_range;
    g_page_alloc.current_offset_pages = 0;
  }

  return 0;
}

void free_page(u64 phys_addr) {
  if (g_page_alloc.initialized == 0 || g_page_alloc.available == 0) {
    return;
  }
  if (phys_addr == 0 || (phys_addr & (PAGE_SIZE - 1ULL)) != 0) {
    return;
  }
  if (g_page_alloc.recycled_count >= MAX_RECYCLED_PAGES || g_page_alloc.free_pages >= g_page_alloc.total_pages) {
    return;
  }
  g_page_alloc.recycled[g_page_alloc.recycled_count++] = phys_addr;
  ++g_page_alloc.free_pages;
}

void page_alloc_stats(page_alloc_stats_t *out_stats) {
  if (out_stats == (page_alloc_stats_t *)0) {
    return;
  }
  out_stats->available = g_page_alloc.available;
  out_stats->total_pages = g_page_alloc.total_pages;
  out_stats->free_pages = g_page_alloc.free_pages;
}
