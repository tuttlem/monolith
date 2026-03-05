#include "arch_mm.h"

#define ARM64_TBL_VALID (1ULL << 0)
#define ARM64_BLK_VALID (1ULL << 0)
#define ARM64_BLK_AF (1ULL << 10)
#define ARM64_BLK_nG (1ULL << 11)
#define ARM64_BLK_SH_INNER (3ULL << 8)
#define ARM64_BLK_SH_OUTER (2ULL << 8)
#define ARM64_BLK_AP_EL0 (1ULL << 6)
#define ARM64_BLK_AP_RO (1ULL << 7)
#define ARM64_BLK_ATTRIDX_SHIFT 2
#define ARM64_BLK_ATTRIDX_NORMAL 0ULL
#define ARM64_BLK_ATTRIDX_DEVICE 1ULL
#define ARM64_BLK_PXN (1ULL << 53)
#define ARM64_BLK_UXN (1ULL << 54)

#define ARM64_L2_SPAN (1ULL << 21)
#define ARM64_IDENTITY_GIB 4ULL
#define ARM64_IDENTITY_BYTES (ARM64_IDENTITY_GIB * 1024ULL * 1024ULL * 1024ULL)
#define ARM64_BLOCK_ADDR_MASK 0x0000FFFFFFE00000ULL

extern u64 g_l0[512];
extern u64 g_l1[512];
extern u64 g_l2[ARM64_IDENTITY_GIB][512];

static u64 block_attr_from_prot(u64 prot_flags) {
  u64 attr = ARM64_BLK_VALID | ARM64_BLK_AF;

  if ((prot_flags & MMU_PROT_DEVICE) != 0) {
    attr |= ARM64_BLK_SH_OUTER;
    attr |= (ARM64_BLK_ATTRIDX_DEVICE << ARM64_BLK_ATTRIDX_SHIFT);
  } else {
    attr |= ARM64_BLK_SH_INNER;
    attr |= (ARM64_BLK_ATTRIDX_NORMAL << ARM64_BLK_ATTRIDX_SHIFT);
  }

  if ((prot_flags & MMU_PROT_USER) != 0) {
    attr |= ARM64_BLK_AP_EL0;
  }
  if ((prot_flags & MMU_PROT_WRITE) == 0) {
    attr |= ARM64_BLK_AP_RO;
  }
  if ((prot_flags & MMU_PROT_EXEC) == 0) {
    attr |= ARM64_BLK_PXN | ARM64_BLK_UXN;
  }
  if ((prot_flags & MMU_PROT_GLOBAL) == 0) {
    attr |= ARM64_BLK_nG;
  }
  return attr;
}

static u64 prot_from_block_attr(u64 entry) {
  u64 flags = MMU_PROT_READ;
  u64 attr_idx = (entry >> ARM64_BLK_ATTRIDX_SHIFT) & 0x7ULL;

  if ((entry & ARM64_BLK_AP_RO) == 0) {
    flags |= MMU_PROT_WRITE;
  }
  if ((entry & ARM64_BLK_AP_EL0) != 0) {
    flags |= MMU_PROT_USER;
  }
  if ((entry & (ARM64_BLK_PXN | ARM64_BLK_UXN)) == 0) {
    flags |= MMU_PROT_EXEC;
  }
  if ((entry & ARM64_BLK_nG) == 0) {
    flags |= MMU_PROT_GLOBAL;
  }
  if (attr_idx == ARM64_BLK_ATTRIDX_DEVICE) {
    flags |= MMU_PROT_DEVICE;
  }
  return flags;
}

static u64 read_ttbr0_el1(void) {
  u64 v;
  __asm__ volatile("mrs %0, ttbr0_el1" : "=r"(v));
  return v;
}

static void write_ttbr0_el1(u64 v) {
  __asm__ volatile("msr ttbr0_el1, %0\n\t"
                   "isb\n\t"
                   :
                   : "r"(v)
                   : "memory");
}

u64 arch_mm_page_size(void) { return ARM64_L2_SPAN; }

status_t arch_mm_map_page(mm_virt_addr_t va, mm_phys_addr_t pa, u64 prot_flags) {
  u64 l1_index;
  u64 l2_index;
  u64 entry;

  if ((va & (ARM64_L2_SPAN - 1ULL)) != 0 || (pa & (ARM64_L2_SPAN - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= ARM64_IDENTITY_BYTES || pa >= ARM64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }
  if ((g_l0[0] & ARM64_TBL_VALID) == 0 || (g_l1[0] & ARM64_TBL_VALID) == 0) {
    return STATUS_FAULT;
  }

  l1_index = (va >> 30) & 0x1FFULL;
  l2_index = (va >> 21) & 0x1FFULL;
  entry = (pa & ARM64_BLOCK_ADDR_MASK) | block_attr_from_prot(prot_flags);
  g_l2[l1_index][l2_index] = entry;
  return STATUS_OK;
}

status_t arch_mm_unmap_page(mm_virt_addr_t va) {
  u64 l1_index;
  u64 l2_index;

  if ((va & (ARM64_L2_SPAN - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= ARM64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  l1_index = (va >> 30) & 0x1FFULL;
  l2_index = (va >> 21) & 0x1FFULL;
  if ((g_l2[l1_index][l2_index] & ARM64_BLK_VALID) == 0) {
    return STATUS_NOT_FOUND;
  }
  g_l2[l1_index][l2_index] = 0;
  return STATUS_OK;
}

status_t arch_mm_protect_page(mm_virt_addr_t va, u64 prot_flags) {
  u64 l1_index;
  u64 l2_index;
  u64 old_entry;
  u64 new_entry;

  if ((va & (ARM64_L2_SPAN - 1ULL)) != 0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= ARM64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  l1_index = (va >> 30) & 0x1FFULL;
  l2_index = (va >> 21) & 0x1FFULL;
  old_entry = g_l2[l1_index][l2_index];
  if ((old_entry & ARM64_BLK_VALID) == 0) {
    return STATUS_NOT_FOUND;
  }

  new_entry = (old_entry & ARM64_BLOCK_ADDR_MASK) | block_attr_from_prot(prot_flags);
  g_l2[l1_index][l2_index] = new_entry;
  return STATUS_OK;
}

status_t arch_mm_translate_page(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags) {
  u64 l1_index;
  u64 l2_index;
  u64 entry;

  if ((va & (ARM64_L2_SPAN - 1ULL)) != 0 || out_pa == (mm_phys_addr_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (va >= ARM64_IDENTITY_BYTES) {
    return STATUS_NOT_SUPPORTED;
  }

  l1_index = (va >> 30) & 0x1FFULL;
  l2_index = (va >> 21) & 0x1FFULL;
  entry = g_l2[l1_index][l2_index];
  if ((entry & ARM64_BLK_VALID) == 0) {
    return STATUS_NOT_FOUND;
  }

  *out_pa = entry & ARM64_BLOCK_ADDR_MASK;
  if (out_flags != (u64 *)0) {
    *out_flags = prot_from_block_attr(entry);
  }
  return STATUS_OK;
}

status_t arch_mm_sync_tlb(mm_virt_addr_t va, u64 size) {
  u64 ttbr0 = read_ttbr0_el1();
  (void)va;
  (void)size;
  __asm__ volatile("dsb ishst" : : : "memory");
  write_ttbr0_el1(ttbr0);
  __asm__ volatile("dsb ish\n\t"
                   "isb\n\t"
                   :
                   :
                   : "memory");
  return STATUS_OK;
}
