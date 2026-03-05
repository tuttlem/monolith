#include "arch/arm64/early_paging.h"

#define ARM64_TBL_VALID (1ULL << 0)
#define ARM64_TBL_TABLE (1ULL << 1)

#define ARM64_BLK_VALID (1ULL << 0)
#define ARM64_BLK_AF (1ULL << 10)
#define ARM64_BLK_SH_INNER (3ULL << 8)
#define ARM64_BLK_SH_OUTER (2ULL << 8)
#define ARM64_BLK_ATTRIDX_SHIFT 2
#define ARM64_BLK_ATTRIDX_NORMAL 0ULL
#define ARM64_BLK_ATTRIDX_DEVICE 1ULL

#define ARM64_L1_SPAN (1ULL << 30)
#define ARM64_L2_SPAN (1ULL << 21)
#define ARM64_IDENTITY_GIB 4ULL

#define ARM64_TTBR_ASID_MASK 0xFFFF000000000000ULL
#define ARM64_TTBR_BADDR_MASK 0x0000FFFFFFFFF000ULL

u64 g_l0[512] __attribute__((aligned(4096)));
u64 g_l1[512] __attribute__((aligned(4096)));
u64 g_l2[ARM64_IDENTITY_GIB][512] __attribute__((aligned(4096)));

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

static void write_mair_el1(u64 v) {
  __asm__ volatile("msr mair_el1, %0\n\t"
                   "isb\n\t"
                   :
                   : "r"(v)
                   : "memory");
}

static void tlb_flush_all_el1(void) {
  __asm__ volatile("dsb ish\n\t"
                   "tlbi vmalle1\n\t"
                   "dsb ish\n\t"
                   "isb\n\t"
                   :
                   :
                   : "memory");
}

static void zero_u64(u64 *p, u64 count) {
  u64 i;
  for (i = 0; i < count; ++i) {
    p[i] = 0;
  }
}

static u32 region_kind_for_phys(const boot_info_t *boot_info, u64 phys) {
  u32 i;
  u32 best_kind = BOOT_MEM_REGION_USABLE;

  if (boot_info == (const boot_info_t *)0) {
    return BOOT_MEM_REGION_USABLE;
  }

  for (i = 0; i < boot_info->memory_region_count; ++i) {
    const boot_mem_region_t *r = &boot_info->memory_regions[i];
    u64 begin = r->base;
    u64 end = r->base + r->size;
    if (r->size == 0) {
      continue;
    }
    if (phys >= begin && phys < end) {
      best_kind = r->kind;
      break;
    }
  }
  return best_kind;
}

static u64 block_attr_for_kind(u32 kind) {
  u64 attr = ARM64_BLK_VALID | ARM64_BLK_AF;

  if (kind == BOOT_MEM_REGION_MMIO) {
    attr |= ARM64_BLK_SH_OUTER;
    attr |= (ARM64_BLK_ATTRIDX_DEVICE << ARM64_BLK_ATTRIDX_SHIFT);
  } else if (kind == BOOT_MEM_REGION_USABLE || kind == BOOT_MEM_REGION_ACPI_RECLAIM || kind == BOOT_MEM_REGION_ACPI_NVS) {
    attr |= ARM64_BLK_SH_INNER;
    attr |= (ARM64_BLK_ATTRIDX_NORMAL << ARM64_BLK_ATTRIDX_SHIFT);
  } else {
    /* Keep non-usable RAM-like regions as normal memory for simple identity execution. */
    attr |= ARM64_BLK_SH_INNER;
    attr |= (ARM64_BLK_ATTRIDX_NORMAL << ARM64_BLK_ATTRIDX_SHIFT);
  }
  return attr;
}

int arm64_early_paging_takeover(const boot_info_t *boot_info, arm64_early_paging_result_t *result) {
  u64 old_ttbr0;
  u64 new_base;
  u64 new_ttbr0;
  u64 i;
  u64 j;

  if (boot_info == (const boot_info_t *)0 || result == (arm64_early_paging_result_t *)0) {
    return 0;
  }

  zero_u64(g_l0, 512ULL);
  zero_u64(g_l1, 512ULL);
  zero_u64(&g_l2[0][0], ARM64_IDENTITY_GIB * 512ULL);

  g_l0[0] = ((u64)(uptr)&g_l1[0] & ARM64_TTBR_BADDR_MASK) | ARM64_TBL_VALID | ARM64_TBL_TABLE;

  for (i = 0; i < ARM64_IDENTITY_GIB; ++i) {
    g_l1[i] = ((u64)(uptr)&g_l2[i][0] & ARM64_TTBR_BADDR_MASK) | ARM64_TBL_VALID | ARM64_TBL_TABLE;

    for (j = 0; j < 512ULL; ++j) {
      u64 phys = (i * ARM64_L1_SPAN) + (j * ARM64_L2_SPAN);
      u32 kind = region_kind_for_phys(boot_info, phys);
      u64 attr = block_attr_for_kind(kind);
      g_l2[i][j] = (phys & ARM64_TTBR_BADDR_MASK) | attr;
    }
  }

  /* MAIR: Attr0 = normal WB RA/WA, Attr1 = device-nGnRnE. */
  write_mair_el1((0xFFULL << 0) | (0x00ULL << 8));

  old_ttbr0 = read_ttbr0_el1();
  new_base = (u64)(uptr)&g_l0[0];
  new_ttbr0 = (old_ttbr0 & ARM64_TTBR_ASID_MASK) | (new_base & ARM64_TTBR_BADDR_MASK);
  write_ttbr0_el1(new_ttbr0);
  tlb_flush_all_el1();

  result->old_ttbr0 = old_ttbr0 & ARM64_TTBR_BADDR_MASK;
  result->new_ttbr0 = new_base & ARM64_TTBR_BADDR_MASK;
  result->identity_bytes_mapped = ARM64_IDENTITY_GIB * 1024ULL * 1024ULL * 1024ULL;
  return 1;
}
