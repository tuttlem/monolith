#ifndef KERNEL_KMALLOC_H
#define KERNEL_KMALLOC_H

#include "kernel.h"

typedef struct {
  u64 available;
  u64 pages;
  u64 bytes_total;
  u64 bytes_used;
  u64 bytes_free;
  u64 alloc_count;
  u64 free_count;
  u64 failed_alloc_count;
  u64 invalid_free_count;
  u64 double_free_count;
} kmalloc_stats_t;

status_t kmalloc_init(boot_info_t *boot_info);
void *kmalloc(u64 size);
void kfree(void *ptr);
void kmalloc_stats(kmalloc_stats_t *out_stats);
int kmalloc_self_test(void);

#endif
