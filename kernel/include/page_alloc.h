#ifndef KERNEL_PAGE_ALLOC_H
#define KERNEL_PAGE_ALLOC_H

#include "kernel.h"

typedef struct {
  BOOT_U64 available;
  BOOT_U64 total_pages;
  BOOT_U64 free_pages;
} page_alloc_stats_t;

status_t page_alloc_init(boot_info_t *boot_info);
BOOT_U64 alloc_page(void);
void free_page(BOOT_U64 phys_addr);
void page_alloc_stats(page_alloc_stats_t *out_stats);

#endif
