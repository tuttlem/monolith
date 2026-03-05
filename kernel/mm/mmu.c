#include "arch_mm.h"
#include "dma.h"
#include "iommu.h"

static int is_aligned(u64 value, u64 alignment) {
  if (alignment == 0) {
    return 0;
  }
  return (value & (alignment - 1ULL)) == 0ULL;
}

u64 mm_page_size(void) {
  u64 page_size = arch_mm_page_size();
  if (page_size == 0) {
    return 4096ULL;
  }
  return page_size;
}

status_t mm_sync_tlb(mm_virt_addr_t va, u64 size) { return arch_mm_sync_tlb(va, size); }

status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, u64 size, u64 prot_flags) {
  u64 page_size = mm_page_size();
  u64 pages;
  u64 i;
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

status_t mm_unmap(mm_virt_addr_t va, u64 size) {
  u64 page_size = mm_page_size();
  u64 pages;
  u64 i;
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

status_t mm_protect(mm_virt_addr_t va, u64 size, u64 prot_flags) {
  u64 page_size = mm_page_size();
  u64 pages;
  u64 i;
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

status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags) {
  u64 page_size = mm_page_size();
  mm_virt_addr_t page_va;
  u64 offset;
  mm_phys_addr_t base_pa;
  u64 flags;
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
  if (out_flags != (u64 *)0) {
    *out_flags = flags;
  }
  return STATUS_OK;
}

static struct {
  u64 initialized;
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

status_t dma_map(u64 device_id, void *cpu_ptr, u64 len, dma_dir_t dir, dma_addr_t *out) {
  u64 addr;
  (void)device_id;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  if (cpu_ptr == (void *)0 || out == (dma_addr_t *)0 || len == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  addr = (u64)(uptr)cpu_ptr;
  if (addr > g_dma.default_constraints.max_addr) {
    return STATUS_NO_MEMORY;
  }
  *out = addr;
  return STATUS_OK;
}

status_t dma_unmap(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir) {
  (void)device_id;
  (void)addr;
  (void)len;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  return STATUS_OK;
}

status_t dma_sync_for_device(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir) {
  (void)device_id;
  (void)addr;
  (void)len;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  return STATUS_OK;
}

status_t dma_sync_for_cpu(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir) {
  (void)device_id;
  (void)addr;
  (void)len;
  (void)dir;
  if (!g_dma.initialized) {
    return STATUS_DEFERRED;
  }
  return STATUS_OK;
}

status_t dma_set_constraints(u64 device_id, const dma_constraints_t *constraints) {
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

status_t dma_get_constraints(u64 device_id, dma_constraints_t *out_constraints) {
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

#define IOMMU_MAX_DOMAINS 16U
#define IOMMU_MAX_MAPS 256U

typedef struct {
  u64 active;
  u64 passthrough;
} iommu_domain_state_t;

typedef struct {
  u64 active;
  iommu_domain_t domain;
  iova_t iova;
  phys_addr_t pa;
  u64 len;
  iommu_perm_t perm;
} iommu_map_entry_t;

static struct {
  u64 initialized;
  iommu_domain_state_t domains[IOMMU_MAX_DOMAINS];
  iommu_map_entry_t maps[IOMMU_MAX_MAPS];
} g_iommu;

status_t iommu_init(const boot_info_t *boot_info) {
  u64 i;
  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  g_iommu.initialized = 1ULL;
  for (i = 0; i < IOMMU_MAX_DOMAINS; ++i) {
    g_iommu.domains[i].active = 0ULL;
    g_iommu.domains[i].passthrough = 1ULL;
  }
  for (i = 0; i < IOMMU_MAX_MAPS; ++i) {
    g_iommu.maps[i].active = 0ULL;
    g_iommu.maps[i].domain = 0ULL;
    g_iommu.maps[i].iova = 0ULL;
    g_iommu.maps[i].pa = 0ULL;
    g_iommu.maps[i].len = 0ULL;
    g_iommu.maps[i].perm = 0ULL;
  }
  return STATUS_OK;
}

status_t iommu_domain_create(iommu_domain_t *out_domain) {
  u64 i;
  if (!g_iommu.initialized) {
    return STATUS_DEFERRED;
  }
  if (out_domain == (iommu_domain_t *)0) {
    return STATUS_INVALID_ARG;
  }
  for (i = 0; i < IOMMU_MAX_DOMAINS; ++i) {
    if (!g_iommu.domains[i].active) {
      g_iommu.domains[i].active = 1ULL;
      g_iommu.domains[i].passthrough = 1ULL;
      *out_domain = i;
      return STATUS_OK;
    }
  }
  return STATUS_NO_MEMORY;
}

status_t iommu_attach(iommu_domain_t domain, u64 device_id) {
  (void)device_id;
  if (!g_iommu.initialized) {
    return STATUS_DEFERRED;
  }
  if (domain >= IOMMU_MAX_DOMAINS || !g_iommu.domains[domain].active) {
    return STATUS_NOT_FOUND;
  }
  return STATUS_OK;
}

status_t iommu_detach(iommu_domain_t domain, u64 device_id) {
  (void)device_id;
  if (!g_iommu.initialized) {
    return STATUS_DEFERRED;
  }
  if (domain >= IOMMU_MAX_DOMAINS || !g_iommu.domains[domain].active) {
    return STATUS_NOT_FOUND;
  }
  return STATUS_OK;
}

status_t iommu_map(iommu_domain_t domain, iova_t iova, phys_addr_t pa, u64 len, iommu_perm_t perm) {
  u64 i;
  if (!g_iommu.initialized) {
    return STATUS_DEFERRED;
  }
  if (domain >= IOMMU_MAX_DOMAINS || !g_iommu.domains[domain].active) {
    return STATUS_NOT_FOUND;
  }
  if (len == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  for (i = 0; i < IOMMU_MAX_MAPS; ++i) {
    if (g_iommu.maps[i].active != 0ULL && g_iommu.maps[i].domain == domain && g_iommu.maps[i].iova == iova) {
      g_iommu.maps[i].pa = pa;
      g_iommu.maps[i].len = len;
      g_iommu.maps[i].perm = perm;
      return STATUS_OK;
    }
  }
  for (i = 0; i < IOMMU_MAX_MAPS; ++i) {
    if (g_iommu.maps[i].active == 0ULL) {
      g_iommu.maps[i].active = 1ULL;
      g_iommu.maps[i].domain = domain;
      g_iommu.maps[i].iova = iova;
      g_iommu.maps[i].pa = pa;
      g_iommu.maps[i].len = len;
      g_iommu.maps[i].perm = perm;
      return STATUS_OK;
    }
  }
  return STATUS_NO_MEMORY;
}

status_t iommu_unmap(iommu_domain_t domain, iova_t iova, u64 len) {
  u64 i;
  if (!g_iommu.initialized) {
    return STATUS_DEFERRED;
  }
  if (domain >= IOMMU_MAX_DOMAINS || !g_iommu.domains[domain].active) {
    return STATUS_NOT_FOUND;
  }
  if (len == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  for (i = 0; i < IOMMU_MAX_MAPS; ++i) {
    if (g_iommu.maps[i].active != 0ULL && g_iommu.maps[i].domain == domain && g_iommu.maps[i].iova == iova) {
      g_iommu.maps[i].active = 0ULL;
      g_iommu.maps[i].domain = 0ULL;
      g_iommu.maps[i].iova = 0ULL;
      g_iommu.maps[i].pa = 0ULL;
      g_iommu.maps[i].len = 0ULL;
      g_iommu.maps[i].perm = 0ULL;
      return STATUS_OK;
    }
  }
  return STATUS_NOT_FOUND;
}

status_t iommu_set_passthrough(iommu_domain_t domain, int enabled) {
  if (!g_iommu.initialized) {
    return STATUS_DEFERRED;
  }
  if (domain >= IOMMU_MAX_DOMAINS || !g_iommu.domains[domain].active) {
    return STATUS_NOT_FOUND;
  }
  g_iommu.domains[domain].passthrough = enabled ? 1ULL : 0ULL;
  return STATUS_OK;
}
