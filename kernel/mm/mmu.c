#include "arch_mm.h"
#include "dma.h"

static int is_aligned(BOOT_U64 value, BOOT_U64 alignment) {
  if (alignment == 0) {
    return 0;
  }
  return (value & (alignment - 1ULL)) == 0ULL;
}

BOOT_U64 mm_page_size(void) {
  BOOT_U64 page_size = arch_mm_page_size();
  if (page_size == 0) {
    return 4096ULL;
  }
  return page_size;
}

status_t mm_sync_tlb(mm_virt_addr_t va, BOOT_U64 size) { return arch_mm_sync_tlb(va, size); }

status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, BOOT_U64 size, BOOT_U64 prot_flags) {
  BOOT_U64 page_size = mm_page_size();
  BOOT_U64 pages;
  BOOT_U64 i;
  status_t st;

  if (size == 0) {
    return STATUS_INVALID_ARG;
  }
  if (!is_aligned(va, page_size) || !is_aligned(pa, page_size) || !is_aligned(size, page_size)) {
    return STATUS_INVALID_ARG;
  }

  pages = size / page_size;
  for (i = 0; i < pages; ++i) {
    mm_virt_addr_t cur_va = va + i * page_size;
    mm_phys_addr_t cur_pa = pa + i * page_size;
    st = arch_mm_map_page(cur_va, cur_pa, prot_flags);
    if (st != STATUS_OK) {
      while (i > 0) {
        --i;
        (void)arch_mm_unmap_page(va + i * page_size);
      }
      (void)arch_mm_sync_tlb(va, size);
      return st;
    }
  }

  return arch_mm_sync_tlb(va, size);
}

status_t mm_unmap(mm_virt_addr_t va, BOOT_U64 size) {
  BOOT_U64 page_size = mm_page_size();
  BOOT_U64 pages;
  BOOT_U64 i;
  status_t st;

  if (size == 0) {
    return STATUS_INVALID_ARG;
  }
  if (!is_aligned(va, page_size) || !is_aligned(size, page_size)) {
    return STATUS_INVALID_ARG;
  }

  pages = size / page_size;
  for (i = 0; i < pages; ++i) {
    st = arch_mm_unmap_page(va + i * page_size);
    if (st != STATUS_OK) {
      if (i != 0) {
        (void)arch_mm_sync_tlb(va, i * page_size);
      }
      return st;
    }
  }

  return arch_mm_sync_tlb(va, size);
}

status_t mm_protect(mm_virt_addr_t va, BOOT_U64 size, BOOT_U64 prot_flags) {
  BOOT_U64 page_size = mm_page_size();
  BOOT_U64 pages;
  BOOT_U64 i;
  status_t st;

  if (size == 0) {
    return STATUS_INVALID_ARG;
  }
  if (!is_aligned(va, page_size) || !is_aligned(size, page_size)) {
    return STATUS_INVALID_ARG;
  }

  pages = size / page_size;
  for (i = 0; i < pages; ++i) {
    st = arch_mm_protect_page(va + i * page_size, prot_flags);
    if (st != STATUS_OK) {
      if (i != 0) {
        (void)arch_mm_sync_tlb(va, i * page_size);
      }
      return st;
    }
  }

  return arch_mm_sync_tlb(va, size);
}

status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, BOOT_U64 *out_flags) {
  BOOT_U64 page_size = mm_page_size();
  mm_virt_addr_t page_va;
  BOOT_U64 offset;
  mm_phys_addr_t base_pa;
  BOOT_U64 flags;
  status_t st;

  if (out_pa == (mm_phys_addr_t *)0) {
    return STATUS_INVALID_ARG;
  }

  page_va = va & ~(page_size - 1ULL);
  offset = va & (page_size - 1ULL);
  st = arch_mm_translate_page(page_va, &base_pa, &flags);
  if (st != STATUS_OK) {
    return st;
  }

  *out_pa = base_pa + offset;
  if (out_flags != (BOOT_U64 *)0) {
    *out_flags = flags;
  }
  return STATUS_OK;
}

static struct {
  BOOT_U64 initialized;
  dma_constraints_t default_constraints;
} g_dma;

status_t dma_init(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  g_dma.initialized = 1ULL;
  g_dma.default_constraints.max_addr = ~0ULL;
  g_dma.default_constraints.max_segment_len = ~0ULL;
  g_dma.default_constraints.alignment = 1ULL;
  return STATUS_OK;
}

status_t dma_map(BOOT_U64 device_id, void *cpu_ptr, BOOT_U64 len, dma_dir_t dir, dma_addr_t *out) {
  BOOT_U64 addr;
  (void)device_id;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  if (cpu_ptr == (void *)0 || out == (dma_addr_t *)0 || len == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  addr = (BOOT_U64)(BOOT_UPTR)cpu_ptr;
  if (addr > g_dma.default_constraints.max_addr) {
    return STATUS_NO_MEMORY;
  }
  *out = addr;
  return STATUS_OK;
}

status_t dma_unmap(BOOT_U64 device_id, dma_addr_t addr, BOOT_U64 len, dma_dir_t dir) {
  (void)device_id;
  (void)addr;
  (void)len;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  return STATUS_OK;
}

status_t dma_sync_for_device(BOOT_U64 device_id, dma_addr_t addr, BOOT_U64 len, dma_dir_t dir) {
  (void)device_id;
  (void)addr;
  (void)len;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  return STATUS_OK;
}

status_t dma_sync_for_cpu(BOOT_U64 device_id, dma_addr_t addr, BOOT_U64 len, dma_dir_t dir) {
  (void)device_id;
  (void)addr;
  (void)len;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  return STATUS_OK;
}

status_t dma_set_constraints(BOOT_U64 device_id, const dma_constraints_t *constraints) {
  (void)device_id;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  if (constraints == (const dma_constraints_t *)0) {
    return STATUS_INVALID_ARG;
  }
  g_dma.default_constraints.max_addr = constraints->max_addr;
  g_dma.default_constraints.max_segment_len = constraints->max_segment_len;
  g_dma.default_constraints.alignment = constraints->alignment;
  if (g_dma.default_constraints.alignment == 0ULL) {
    g_dma.default_constraints.alignment = 1ULL;
  }
  return STATUS_OK;
}

status_t dma_get_constraints(BOOT_U64 device_id, dma_constraints_t *out_constraints) {
  (void)device_id;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  if (out_constraints == (dma_constraints_t *)0) {
    return STATUS_INVALID_ARG;
  }
  out_constraints->max_addr = g_dma.default_constraints.max_addr;
  out_constraints->max_segment_len = g_dma.default_constraints.max_segment_len;
  out_constraints->alignment = g_dma.default_constraints.alignment;
  return STATUS_OK;
}
