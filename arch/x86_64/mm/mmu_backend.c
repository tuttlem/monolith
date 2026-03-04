#include "arch_mm.h"

#define X64_PAGE_PRESENT (1ULL << 0)
#define X64_PAGE_WRITABLE (1ULL << 1)
#define X64_PAGE_USER (1ULL << 2)
#define X64_PAGE_PWT (1ULL << 3)
#define X64_PAGE_PCD (1ULL << 4)
#define X64_PAGE_LARGE (1ULL << 7)
#define X64_PAGE_GLOBAL (1ULL << 8)
#define X64_PAGE_NX (1ULL << 63)

#define X64_ENTRY_COUNT 512ULL
#define X64_PAGE_SIZE_2M (2ULL * 1024ULL * 1024ULL)
#define X64_IDENTITY_GIB 4ULL
#define X64_IDENTITY_BYTES (X64_IDENTITY_GIB * 1024ULL * 1024ULL * 1024ULL)
#define X64_ADDR_MASK_2M 0x000FFFFFFFE00000ULL

extern BOOT_U64 g_pml4[512];
extern BOOT_U64 g_pdpt[512];
extern BOOT_U64 g_pd[X64_IDENTITY_GIB][512];

static BOOT_U64 read_cr3(void) {
  BOOT_U64 cr3;
  __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
  return cr3;
}

static void write_cr3(BOOT_U64 cr3) { __asm__ volatile("movq %0, %%cr3" : : "r"(cr3) : "memory"); }

static int range_supported(mm_virt_addr_t va, mm_phys_addr_t pa) {
  if (va >= X64_IDENTITY_BYTES) {
    return 0;
  }
  if (pa >= X64_IDENTITY_BYTES) {
    return 0;
  }
  return 1;
}

static BOOT_U64 x64_flags_from_prot(BOOT_U64 prot_flags) {
  BOOT_U64 flags = X64_PAGE_PRESENT | X64_PAGE_LARGE;

  if ((prot_flags & MMU_PROT_WRITE) != 0) {
    flags |= X64_PAGE_WRITABLE;
  }
  if ((prot_flags & MMU_PROT_USER) != 0) {
    flags |= X64_PAGE_USER;
  }
  if ((prot_flags & MMU_PROT_GLOBAL) != 0) {
    flags |= X64_PAGE_GLOBAL;
  }
  if ((prot_flags & MMU_PROT_DEVICE) != 0) {
    flags |= X64_PAGE_PCD | X64_PAGE_PWT;
  }
  if ((prot_flags & MMU_PROT_EXEC) == 0) {
    flags |= X64_PAGE_NX;
  }
  return flags;
}

static BOOT_U64 x64_prot_from_entry(BOOT_U64 entry) {
  BOOT_U64 flags = MMU_PROT_READ;

  if ((entry & X64_PAGE_WRITABLE) != 0) {
    flags |= MMU_PROT_WRITE;
  }
  if ((entry & X64_PAGE_USER) != 0) {
    flags |= MMU_PROT_USER;
  }
  if ((entry & X64_PAGE_GLOBAL) != 0) {
    flags |= MMU_PROT_GLOBAL;
  }
  if ((entry & (X64_PAGE_PCD | X64_PAGE_PWT)) != 0) {
    flags |= MMU_PROT_DEVICE;
  }
  if ((entry & X64_PAGE_NX) == 0) {
    flags |= MMU_PROT_EXEC;
  }
  return flags;
}

BOOT_U64 arch_mm_page_size(void) { return X64_PAGE_SIZE_2M; }

status_t arch_mm_map_page(mm_virt_addr_t va, mm_phys_addr_t pa, BOOT_U64 prot_flags) {
  BOOT_U64 pdpt_index;
  BOOT_U64 pd_index;
  BOOT_U64 entry;

  if ((va & (X64_PAGE_SIZE_2M - 1ULL)) != 0 || (pa & (X64_PAGE_SIZE_2M - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (!range_supported(va, pa)) {
    return STATUS_NOT_SUPPORTED;
  }
  if ((g_pml4[0] & X64_PAGE_PRESENT) == 0 || (g_pdpt[0] & X64_PAGE_PRESENT) == 0) {
    return STATUS_FAULT;
  }

  pdpt_index = (va >> 30) & 0x1FFULL;
  pd_index = (va >> 21) & 0x1FFULL;
  if ((prot_flags & MMU_PROT_USER) != 0ULL) {
    g_pml4[0] |= X64_PAGE_USER;
    g_pdpt[pdpt_index] |= X64_PAGE_USER;
  }
  entry = (pa & X64_ADDR_MASK_2M) | x64_flags_from_prot(prot_flags);
  g_pd[pdpt_index][pd_index] = entry;
  return STATUS_OK;
}

status_t arch_mm_unmap_page(mm_virt_addr_t va) {
  BOOT_U64 pdpt_index;
  BOOT_U64 pd_index;

  if ((va & (X64_PAGE_SIZE_2M - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= X64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  pdpt_index = (va >> 30) & 0x1FFULL;
  pd_index = (va >> 21) & 0x1FFULL;
  if ((g_pd[pdpt_index][pd_index] & X64_PAGE_PRESENT) == 0) {
    return STATUS_NOT_FOUND;
  }
  g_pd[pdpt_index][pd_index] = 0;
  return STATUS_OK;
}

status_t arch_mm_protect_page(mm_virt_addr_t va, BOOT_U64 prot_flags) {
  BOOT_U64 pdpt_index;
  BOOT_U64 pd_index;
  BOOT_U64 old_entry;
  BOOT_U64 new_entry;

  if ((va & (X64_PAGE_SIZE_2M - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= X64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  pdpt_index = (va >> 30) & 0x1FFULL;
  pd_index = (va >> 21) & 0x1FFULL;
  if ((prot_flags & MMU_PROT_USER) != 0ULL) {
    g_pml4[0] |= X64_PAGE_USER;
    g_pdpt[pdpt_index] |= X64_PAGE_USER;
  }
  old_entry = g_pd[pdpt_index][pd_index];
  if ((old_entry & X64_PAGE_PRESENT) == 0) {
    return STATUS_NOT_FOUND;
  }

  new_entry = (old_entry & X64_ADDR_MASK_2M) | x64_flags_from_prot(prot_flags);
  g_pd[pdpt_index][pd_index] = new_entry;
  return STATUS_OK;
}

status_t arch_mm_translate_page(mm_virt_addr_t va, mm_phys_addr_t *out_pa, BOOT_U64 *out_flags) {
  BOOT_U64 pdpt_index;
  BOOT_U64 pd_index;
  BOOT_U64 entry;

  if ((va & (X64_PAGE_SIZE_2M - 1ULL)) != 0 || out_pa == (mm_phys_addr_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= X64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  pdpt_index = (va >> 30) & 0x1FFULL;
  pd_index = (va >> 21) & 0x1FFULL;
  entry = g_pd[pdpt_index][pd_index];
  if ((entry & X64_PAGE_PRESENT) == 0) {
    return STATUS_NOT_FOUND;
  }

  *out_pa = entry & X64_ADDR_MASK_2M;
  if (out_flags != (BOOT_U64 *)0) {
    *out_flags = x64_prot_from_entry(entry);
  }
  return STATUS_OK;
}

status_t arch_mm_sync_tlb(mm_virt_addr_t va, BOOT_U64 size) {
  BOOT_U64 cr3;
  (void)va;
  (void)size;

  cr3 = read_cr3();
  write_cr3(cr3);
  return STATUS_OK;
}
