#include "kmalloc.h"
#include "page_alloc.h"

#define KMALLOC_PAGE_SIZE 4096ULL
#define KMALLOC_ALIGN 16ULL
#define KMALLOC_PAGE_MAGIC 0x4B4D504147454D47ULL
#define KMALLOC_BLOCK_MAGIC 0x4B4D424C4F434B31ULL

#define ALIGN_UP(v, a) (((v) + ((a)-1ULL)) & ~((a)-1ULL))

typedef struct kmalloc_block kmalloc_block_t;
typedef struct kmalloc_page kmalloc_page_t;

struct kmalloc_block {
  BOOT_U64 magic;
  BOOT_U64 size;
  BOOT_U64 free;
  BOOT_U64 reserved;
  kmalloc_block_t *prev;
  kmalloc_block_t *next;
};

struct kmalloc_page {
  BOOT_U64 magic;
  BOOT_U64 phys_addr;
  BOOT_U64 free_bytes;
  BOOT_U64 reserved;
  kmalloc_page_t *prev;
  kmalloc_page_t *next;
  kmalloc_block_t *first;
};

#define KMALLOC_PAGE_HEADER_SIZE ALIGN_UP(sizeof(kmalloc_page_t), KMALLOC_ALIGN)
#define KMALLOC_BLOCK_HEADER_SIZE ALIGN_UP(sizeof(kmalloc_block_t), KMALLOC_ALIGN)
#define KMALLOC_MIN_SPLIT_PAYLOAD KMALLOC_ALIGN
#define KMALLOC_FIRST_BLOCK_PAYLOAD (KMALLOC_PAGE_SIZE - KMALLOC_PAGE_HEADER_SIZE - KMALLOC_BLOCK_HEADER_SIZE)

static struct {
  BOOT_U64 initialized;
  BOOT_U64 available;
  BOOT_U64 pages;
  BOOT_U64 bytes_used;
  BOOT_U64 alloc_count;
  BOOT_U64 free_count;
  BOOT_U64 failed_alloc_count;
  BOOT_U64 invalid_free_count;
  BOOT_U64 double_free_count;
  kmalloc_page_t *head;
} g_kmalloc;

static kmalloc_page_t *alloc_kmalloc_page(void) {
  BOOT_U64 phys = alloc_page();
  kmalloc_page_t *page;
  kmalloc_block_t *block;
  unsigned char *base;

  if (phys == 0) {
    return (kmalloc_page_t *)0;
  }

  base = (unsigned char *)(BOOT_UPTR)phys;
  page = (kmalloc_page_t *)(void *)base;
  page->magic = KMALLOC_PAGE_MAGIC;
  page->phys_addr = phys;
  page->free_bytes = KMALLOC_FIRST_BLOCK_PAYLOAD;
  page->reserved = 0;
  page->prev = (kmalloc_page_t *)0;
  page->next = (kmalloc_page_t *)0;

  block = (kmalloc_block_t *)(void *)(base + KMALLOC_PAGE_HEADER_SIZE);
  block->magic = KMALLOC_BLOCK_MAGIC;
  block->size = KMALLOC_FIRST_BLOCK_PAYLOAD;
  block->free = 1;
  block->reserved = 0;
  block->prev = (kmalloc_block_t *)0;
  block->next = (kmalloc_block_t *)0;
  page->first = block;

  ++g_kmalloc.pages;
  return page;
}

static void insert_page(kmalloc_page_t *page) {
  if (page == (kmalloc_page_t *)0) {
    return;
  }
  page->prev = (kmalloc_page_t *)0;
  page->next = g_kmalloc.head;
  if (g_kmalloc.head != (kmalloc_page_t *)0) {
    g_kmalloc.head->prev = page;
  }
  g_kmalloc.head = page;
}

static void remove_page(kmalloc_page_t *page) {
  if (page == (kmalloc_page_t *)0) {
    return;
  }
  if (page->prev != (kmalloc_page_t *)0) {
    page->prev->next = page->next;
  } else {
    g_kmalloc.head = page->next;
  }
  if (page->next != (kmalloc_page_t *)0) {
    page->next->prev = page->prev;
  }
  page->prev = (kmalloc_page_t *)0;
  page->next = (kmalloc_page_t *)0;
}

static kmalloc_page_t *find_page_for_ptr(void *ptr) {
  kmalloc_page_t *page = g_kmalloc.head;
  BOOT_U64 p = (BOOT_U64)(BOOT_UPTR)ptr;

  while (page != (kmalloc_page_t *)0) {
    BOOT_U64 base = page->phys_addr;
    BOOT_U64 end = base + KMALLOC_PAGE_SIZE;
    if (p >= base && p < end) {
      return page;
    }
    page = page->next;
  }
  return (kmalloc_page_t *)0;
}

static void split_block(kmalloc_page_t *page, kmalloc_block_t *block, BOOT_U64 want_size) {
  BOOT_U64 remaining;
  unsigned char *new_addr;
  kmalloc_block_t *new_block;

  if (page == (kmalloc_page_t *)0 || block == (kmalloc_block_t *)0) {
    return;
  }
  if (block->size < want_size) {
    return;
  }

  remaining = block->size - want_size;
  if (remaining < (KMALLOC_BLOCK_HEADER_SIZE + KMALLOC_MIN_SPLIT_PAYLOAD)) {
    return;
  }

  new_addr = (unsigned char *)(void *)block + KMALLOC_BLOCK_HEADER_SIZE + want_size;
  new_block = (kmalloc_block_t *)(void *)new_addr;
  new_block->magic = KMALLOC_BLOCK_MAGIC;
  new_block->size = remaining - KMALLOC_BLOCK_HEADER_SIZE;
  new_block->free = 1;
  new_block->reserved = 0;
  new_block->prev = block;
  new_block->next = block->next;
  if (block->next != (kmalloc_block_t *)0) {
    block->next->prev = new_block;
  }
  block->next = new_block;
  block->size = want_size;
  page->free_bytes -= KMALLOC_BLOCK_HEADER_SIZE;
}

static void coalesce_forward(kmalloc_page_t *page, kmalloc_block_t *block) {
  kmalloc_block_t *next;

  if (page == (kmalloc_page_t *)0 || block == (kmalloc_block_t *)0) {
    return;
  }

  next = block->next;
  if (next == (kmalloc_block_t *)0 || next->free == 0 || next->magic != KMALLOC_BLOCK_MAGIC) {
    return;
  }

  block->size += KMALLOC_BLOCK_HEADER_SIZE + next->size;
  block->next = next->next;
  if (next->next != (kmalloc_block_t *)0) {
    next->next->prev = block;
  }
  page->free_bytes += KMALLOC_BLOCK_HEADER_SIZE;
}

void kmalloc_init(boot_info_t *boot_info) {
  page_alloc_stats_t page_stats;

  g_kmalloc.initialized = 1;
  g_kmalloc.available = 0;
  g_kmalloc.pages = 0;
  g_kmalloc.bytes_used = 0;
  g_kmalloc.alloc_count = 0;
  g_kmalloc.free_count = 0;
  g_kmalloc.failed_alloc_count = 0;
  g_kmalloc.invalid_free_count = 0;
  g_kmalloc.double_free_count = 0;
  g_kmalloc.head = (kmalloc_page_t *)0;

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
  BOOT_U64 want_size;
  kmalloc_page_t *page;
  kmalloc_block_t *block;

  if (g_kmalloc.initialized == 0 || g_kmalloc.available == 0 || size == 0) {
    return (void *)0;
  }

  want_size = ALIGN_UP(size, KMALLOC_ALIGN);
  if (want_size > KMALLOC_FIRST_BLOCK_PAYLOAD) {
    ++g_kmalloc.failed_alloc_count;
    return (void *)0;
  }

  page = g_kmalloc.head;
  while (page != (kmalloc_page_t *)0) {
    if (page->magic == KMALLOC_PAGE_MAGIC && page->free_bytes >= want_size) {
      block = page->first;
      while (block != (kmalloc_block_t *)0) {
        if (block->magic == KMALLOC_BLOCK_MAGIC && block->free != 0 && block->size >= want_size) {
          split_block(page, block, want_size);
          block->free = 0;
          page->free_bytes -= block->size;
          g_kmalloc.bytes_used += block->size;
          ++g_kmalloc.alloc_count;
          return (void *)((unsigned char *)(void *)block + KMALLOC_BLOCK_HEADER_SIZE);
        }
        block = block->next;
      }
    }
    page = page->next;
  }

  page = alloc_kmalloc_page();
  if (page == (kmalloc_page_t *)0) {
    ++g_kmalloc.failed_alloc_count;
    return (void *)0;
  }
  insert_page(page);

  block = page->first;
  split_block(page, block, want_size);
  block->free = 0;
  page->free_bytes -= block->size;
  g_kmalloc.bytes_used += block->size;
  ++g_kmalloc.alloc_count;
  return (void *)((unsigned char *)(void *)block + KMALLOC_BLOCK_HEADER_SIZE);
}

void kfree(void *ptr) {
  kmalloc_page_t *page;
  kmalloc_block_t *block;
  kmalloc_block_t *prev;

  if (ptr == (void *)0 || g_kmalloc.initialized == 0 || g_kmalloc.available == 0) {
    return;
  }

  if (((BOOT_U64)(BOOT_UPTR)ptr & (KMALLOC_ALIGN - 1ULL)) != 0) {
    ++g_kmalloc.invalid_free_count;
    return;
  }

  page = find_page_for_ptr(ptr);
  if (page == (kmalloc_page_t *)0 || page->magic != KMALLOC_PAGE_MAGIC) {
    ++g_kmalloc.invalid_free_count;
    return;
  }

  block = (kmalloc_block_t *)(void *)((unsigned char *)ptr - KMALLOC_BLOCK_HEADER_SIZE);
  if ((BOOT_U64)(BOOT_UPTR)block < page->phys_addr + KMALLOC_PAGE_HEADER_SIZE ||
      (BOOT_U64)(BOOT_UPTR)block >= page->phys_addr + KMALLOC_PAGE_SIZE ||
      block->magic != KMALLOC_BLOCK_MAGIC) {
    ++g_kmalloc.invalid_free_count;
    return;
  }

  if (block->free != 0) {
    ++g_kmalloc.double_free_count;
    return;
  }

  block->free = 1;
  page->free_bytes += block->size;
  g_kmalloc.bytes_used -= block->size;
  ++g_kmalloc.free_count;

  coalesce_forward(page, block);
  prev = block->prev;
  if (prev != (kmalloc_block_t *)0 && prev->free != 0 && prev->magic == KMALLOC_BLOCK_MAGIC) {
    coalesce_forward(page, prev);
    block = prev;
  }

  if (page->free_bytes == KMALLOC_FIRST_BLOCK_PAYLOAD && block == page->first && block->next == (kmalloc_block_t *)0 &&
      block->free != 0) {
    remove_page(page);
    free_page(page->phys_addr);
    --g_kmalloc.pages;
  }
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
  out_stats->alloc_count = g_kmalloc.alloc_count;
  out_stats->free_count = g_kmalloc.free_count;
  out_stats->failed_alloc_count = g_kmalloc.failed_alloc_count;
  out_stats->invalid_free_count = g_kmalloc.invalid_free_count;
  out_stats->double_free_count = g_kmalloc.double_free_count;
}

int kmalloc_self_test(void) {
  void *a[10];
  void *b[5];
  kmalloc_stats_t before;
  kmalloc_stats_t after;
  BOOT_U32 i;

  kmalloc_stats(&before);
  if (before.available == 0) {
    return 1;
  }

  for (i = 0; i < 10U; ++i) {
    a[i] = kmalloc(64);
    if (a[i] == (void *)0) {
      return 0;
    }
  }
  for (i = 0; i < 5U; ++i) {
    kfree(a[i]);
  }
  for (i = 0; i < 5U; ++i) {
    b[i] = kmalloc(64);
    if (b[i] == (void *)0) {
      return 0;
    }
  }
  for (i = 5U; i < 10U; ++i) {
    kfree(a[i]);
  }
  for (i = 0; i < 5U; ++i) {
    kfree(b[i]);
  }

  kmalloc_stats(&after);
  if (after.bytes_used != before.bytes_used) {
    return 0;
  }
  return 1;
}
