#ifndef KERNEL_PAGE_ALLOC_H
#define KERNEL_PAGE_ALLOC_H

#include "kernel.h"

typedef struct {
  u64 available;
  u64 total_pages;
  u64 free_pages;
} page_alloc_stats_t;

status_t page_alloc_init(boot_info_t *boot_info);
u64 alloc_page(void);
void free_page(u64 phys_addr);
void page_alloc_stats(page_alloc_stats_t *out_stats);

#endif
