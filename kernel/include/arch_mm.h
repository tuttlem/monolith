#ifndef KERNEL_ARCH_MM_H
#define KERNEL_ARCH_MM_H

#include "memory_init.h"

#define ARCH_MM_API_VERSION_MAJOR 1U
#define ARCH_MM_API_VERSION_MINOR 0U

typedef u64 mm_phys_addr_t;
typedef u64 mm_virt_addr_t;

typedef enum {
  MMU_PROT_READ = 1ULL << 0,
  MMU_PROT_WRITE = 1ULL << 1,
  MMU_PROT_EXEC = 1ULL << 2,
  MMU_PROT_USER = 1ULL << 3,
  MMU_PROT_DEVICE = 1ULL << 4,
  MMU_PROT_GLOBAL = 1ULL << 5
} mmu_prot_t;

/*
 * Stable MM HAL interface.
 * The existing arch_memory_init() path is retained as the backend-compatible
 * implementation for early MMU takeover.
 */
static inline status_t arch_mm_early_init(boot_info_t *boot_info) {
  return arch_memory_init(boot_info);
}

/* Generic MMU API for kernel code. */
/* Partial-failure behavior:
 * - mm_map() rolls back pages already mapped in the current call.
 * - mm_unmap()/mm_protect() stop at first failing page and keep prior updates.
 */
status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, u64 size, u64 prot_flags);
status_t mm_unmap(mm_virt_addr_t va, u64 size);
status_t mm_protect(mm_virt_addr_t va, u64 size, u64 prot_flags);
status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags);
status_t mm_sync_tlb(mm_virt_addr_t va, u64 size);
u64 mm_page_size(void);

/*
 * Backend contract implemented by each architecture:
 * - map/unmap/protect operate on one backend page granule.
 * - translate returns mapping for a single virtual address.
 */
u64 arch_mm_page_size(void);
status_t arch_mm_map_page(mm_virt_addr_t va, mm_phys_addr_t pa, u64 prot_flags);
status_t arch_mm_unmap_page(mm_virt_addr_t va);
status_t arch_mm_protect_page(mm_virt_addr_t va, u64 prot_flags);
status_t arch_mm_translate_page(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags);
status_t arch_mm_sync_tlb(mm_virt_addr_t va, u64 size);

#endif
