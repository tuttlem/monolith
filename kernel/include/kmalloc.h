#ifndef KERNEL_KMALLOC_H
#define KERNEL_KMALLOC_H

#include "boot_info.h"

typedef struct {
  BOOT_U64 available;
  BOOT_U64 pages;
  BOOT_U64 bytes_total;
  BOOT_U64 bytes_used;
  BOOT_U64 bytes_free;
} kmalloc_stats_t;

void kmalloc_init(boot_info_t *boot_info);
void *kmalloc(BOOT_U64 size);
void kfree(void *ptr);
void kmalloc_stats(kmalloc_stats_t *out_stats);

#endif
