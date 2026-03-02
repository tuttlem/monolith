#include "arch_mm.h"

#define RISCV64_PTE_V (1ULL << 0)
#define RISCV64_PTE_R (1ULL << 1)
#define RISCV64_PTE_W (1ULL << 2)
#define RISCV64_PTE_X (1ULL << 3)
#define RISCV64_PTE_U (1ULL << 4)
#define RISCV64_PTE_G (1ULL << 5)
#define RISCV64_PTE_A (1ULL << 6)
#define RISCV64_PTE_D (1ULL << 7)

#define RISCV64_GIB (1024ULL * 1024ULL * 1024ULL)
#define RISCV64_IDENTITY_GIB 4ULL
#define RISCV64_IDENTITY_BYTES (RISCV64_IDENTITY_GIB * RISCV64_GIB)
#define RISCV64_PPN_MASK ((1ULL << 44) - 1ULL)

extern BOOT_U64 g_root[512];

static BOOT_U64 pte_from_prot(mm_phys_addr_t pa, BOOT_U64 prot_flags) {
  BOOT_U64 ppn = (pa >> 12) & RISCV64_PPN_MASK;
  BOOT_U64 pte = (ppn << 10) | RISCV64_PTE_V | RISCV64_PTE_A;

  if ((prot_flags & MMU_PROT_WRITE) != 0) {
    pte |= RISCV64_PTE_W | RISCV64_PTE_D;
  }
  if ((prot_flags & MMU_PROT_EXEC) != 0) {
    pte |= RISCV64_PTE_X;
  }
  if ((prot_flags & MMU_PROT_READ) != 0 || (prot_flags & MMU_PROT_WRITE) != 0 || (prot_flags & MMU_PROT_EXEC) != 0) {
    pte |= RISCV64_PTE_R;
  }
  if ((prot_flags & MMU_PROT_USER) != 0) {
    pte |= RISCV64_PTE_U;
  }
  if ((prot_flags & MMU_PROT_GLOBAL) != 0) {
    pte |= RISCV64_PTE_G;
  }
  return pte;
}

static BOOT_U64 prot_from_pte(BOOT_U64 pte) {
  BOOT_U64 flags = 0;

  if ((pte & RISCV64_PTE_R) != 0) {
    flags |= MMU_PROT_READ;
  }
  if ((pte & RISCV64_PTE_W) != 0) {
    flags |= MMU_PROT_WRITE;
  }
  if ((pte & RISCV64_PTE_X) != 0) {
    flags |= MMU_PROT_EXEC;
  }
  if ((pte & RISCV64_PTE_U) != 0) {
    flags |= MMU_PROT_USER;
  }
  if ((pte & RISCV64_PTE_G) != 0) {
    flags |= MMU_PROT_GLOBAL;
  }
  return flags;
}

static void sfence_vma_all(void) { __asm__ volatile("sfence.vma x0, x0" : : : "memory"); }

BOOT_U64 arch_mm_page_size(void) { return RISCV64_GIB; }

status_t arch_mm_map_page(mm_virt_addr_t va, mm_phys_addr_t pa, BOOT_U64 prot_flags) {
  BOOT_U64 idx;

  if ((va & (RISCV64_GIB - 1ULL)) != 0 || (pa & (RISCV64_GIB - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= RISCV64_IDENTITY_BYTES || pa >= RISCV64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  idx = (va >> 30) & 0x1FFULL;
  g_root[idx] = pte_from_prot(pa, prot_flags);
  return STATUS_OK;
}

status_t arch_mm_unmap_page(mm_virt_addr_t va) {
  BOOT_U64 idx;

  if ((va & (RISCV64_GIB - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= RISCV64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  idx = (va >> 30) & 0x1FFULL;
  if ((g_root[idx] & RISCV64_PTE_V) == 0) {
    return STATUS_NOT_FOUND;
  }
  g_root[idx] = 0;
  return STATUS_OK;
}

status_t arch_mm_protect_page(mm_virt_addr_t va, BOOT_U64 prot_flags) {
  BOOT_U64 idx;
  BOOT_U64 old_pte;
  mm_phys_addr_t pa;

  if ((va & (RISCV64_GIB - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= RISCV64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  idx = (va >> 30) & 0x1FFULL;
  old_pte = g_root[idx];
  if ((old_pte & RISCV64_PTE_V) == 0) {
    return STATUS_NOT_FOUND;
  }

  pa = ((old_pte >> 10) & RISCV64_PPN_MASK) << 12;
  g_root[idx] = pte_from_prot(pa, prot_flags);
  return STATUS_OK;
}

status_t arch_mm_translate_page(mm_virt_addr_t va, mm_phys_addr_t *out_pa, BOOT_U64 *out_flags) {
  BOOT_U64 idx;
  BOOT_U64 pte;

  if ((va & (RISCV64_GIB - 1ULL)) != 0 || out_pa == (mm_phys_addr_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= RISCV64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  idx = (va >> 30) & 0x1FFULL;
  pte = g_root[idx];
  if ((pte & RISCV64_PTE_V) == 0) {
    return STATUS_NOT_FOUND;
  }

  *out_pa = ((pte >> 10) & RISCV64_PPN_MASK) << 12;
  if (out_flags != (BOOT_U64 *)0) {
    *out_flags = prot_from_pte(pte);
  }
  return STATUS_OK;
}

status_t arch_mm_sync_tlb(mm_virt_addr_t va, BOOT_U64 size) {
  (void)va;
  (void)size;
  sfence_vma_all();
  return STATUS_OK;
}
