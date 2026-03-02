#include "arch_mm.h"

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
