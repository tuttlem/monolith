#ifndef KERNEL_KMALLOC_H
#define KERNEL_KMALLOC_H

#include "kernel.h"

typedef struct {
  BOOT_U64 available;
  BOOT_U64 pages;
  BOOT_U64 bytes_total;
  BOOT_U64 bytes_used;
  BOOT_U64 bytes_free;
  BOOT_U64 alloc_count;
  BOOT_U64 free_count;
  BOOT_U64 failed_alloc_count;
  BOOT_U64 invalid_free_count;
  BOOT_U64 double_free_count;
} kmalloc_stats_t;

status_t kmalloc_init(boot_info_t *boot_info);
void *kmalloc(BOOT_U64 size);
void kfree(void *ptr);
void kmalloc_stats(kmalloc_stats_t *out_stats);
int kmalloc_self_test(void);

#endif
