#include "kmalloc.h"
#include "page_alloc.h"

#define KMALLOC_PAGE_SIZE 4096ULL
#define KMALLOC_ALIGN 16ULL

static struct {
  BOOT_U64 initialized;
  BOOT_U64 available;
  BOOT_U64 pages;
  BOOT_U64 bytes_used;
  BOOT_U64 current_ptr;
  BOOT_U64 current_remaining;
} g_kmalloc;

static BOOT_U64 align_up(BOOT_U64 value, BOOT_U64 align) {
  return (value + (align - 1ULL)) & ~(align - 1ULL);
}

void kmalloc_init(boot_info_t *boot_info) {
  page_alloc_stats_t page_stats;

  g_kmalloc.initialized = 1;
  g_kmalloc.available = 0;
  g_kmalloc.pages = 0;
  g_kmalloc.bytes_used = 0;
  g_kmalloc.current_ptr = 0;
  g_kmalloc.current_remaining = 0;

  if (boot_info == (boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return;
  }

  page_alloc_stats(&page_stats);
  if (page_stats.available == 0 || page_stats.free_pages == 0) {
    return;
  }

  g_kmalloc.available = 1;
}

void *kmalloc(BOOT_U64 size) {
  BOOT_U64 aligned_size;
  BOOT_U64 page_addr;

  if (g_kmalloc.initialized == 0 || g_kmalloc.available == 0 || size == 0) {
    return (void *)0;
  }

  aligned_size = align_up(size, KMALLOC_ALIGN);
  if (aligned_size > (KMALLOC_PAGE_SIZE - KMALLOC_ALIGN)) {
    return (void *)0;
  }

  if (g_kmalloc.current_remaining < aligned_size) {
    page_addr = alloc_page();
    if (page_addr == 0) {
      return (void *)0;
    }
    g_kmalloc.current_ptr = align_up(page_addr, KMALLOC_ALIGN);
    g_kmalloc.current_remaining = KMALLOC_PAGE_SIZE - (g_kmalloc.current_ptr - page_addr);
    ++g_kmalloc.pages;
  }

  page_addr = g_kmalloc.current_ptr;
  g_kmalloc.current_ptr += aligned_size;
  g_kmalloc.current_remaining -= aligned_size;
  g_kmalloc.bytes_used += aligned_size;
  return (void *)(BOOT_UPTR)page_addr;
}

void kfree(void *ptr) {
  (void)ptr;
  /* Bootstrap bump allocator: individual frees are intentionally ignored. */
}

void kmalloc_stats(kmalloc_stats_t *out_stats) {
  if (out_stats == (kmalloc_stats_t *)0) {
    return;
  }

  out_stats->available = g_kmalloc.available;
  out_stats->pages = g_kmalloc.pages;
  out_stats->bytes_total = g_kmalloc.pages * KMALLOC_PAGE_SIZE;
  out_stats->bytes_used = g_kmalloc.bytes_used;
  out_stats->bytes_free = out_stats->bytes_total - out_stats->bytes_used;
}
