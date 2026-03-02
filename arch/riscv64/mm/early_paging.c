#include "arch/riscv64/early_paging.h"

#define RISCV64_PTE_V (1ULL << 0)
#define RISCV64_PTE_R (1ULL << 1)
#define RISCV64_PTE_W (1ULL << 2)
#define RISCV64_PTE_X (1ULL << 3)
#define RISCV64_PTE_A (1ULL << 6)
#define RISCV64_PTE_D (1ULL << 7)

#define RISCV64_SATP_MODE_SHIFT 60ULL
#define RISCV64_SATP_MODE_MASK (0xFULL << RISCV64_SATP_MODE_SHIFT)
#define RISCV64_SATP_PPN_MASK ((1ULL << 44) - 1ULL)
#define RISCV64_SATP_MODE_SV39 8ULL

#define RISCV64_GIB (1024ULL * 1024ULL * 1024ULL)
#define RISCV64_IDENTITY_GIB 4ULL

BOOT_U64 g_root[512] __attribute__((aligned(4096)));

static BOOT_U64 read_satp(void) {
  BOOT_U64 v;
  __asm__ volatile("csrr %0, satp" : "=r"(v));
  return v;
}

static void write_satp(BOOT_U64 v) { __asm__ volatile("csrw satp, %0" : : "r"(v) : "memory"); }

static void sfence_vma_all(void) { __asm__ volatile("sfence.vma x0, x0" : : : "memory"); }

static void zero_u64(BOOT_U64 *p, BOOT_U64 count) {
  BOOT_U64 i;
  for (i = 0; i < count; ++i) {
    p[i] = 0;
  }
}

int riscv64_early_paging_takeover(riscv64_early_paging_result_t *result) {
  BOOT_U64 i;
  BOOT_U64 old_satp;
  BOOT_U64 new_satp;
  BOOT_U64 root_ppn;
  BOOT_U64 mode;

  if (result == (riscv64_early_paging_result_t *)0) {
    return 0;
  }

  zero_u64(g_root, 512ULL);

  for (i = 0; i < RISCV64_IDENTITY_GIB; ++i) {
    BOOT_U64 phys = i * RISCV64_GIB;
    BOOT_U64 ppn = phys >> 12;
    BOOT_U64 leaf = (ppn << 10) | RISCV64_PTE_V | RISCV64_PTE_R | RISCV64_PTE_W | RISCV64_PTE_X | RISCV64_PTE_A |
                    RISCV64_PTE_D;
    g_root[i] = leaf;
  }

  old_satp = read_satp();
  mode = (old_satp & RISCV64_SATP_MODE_MASK) >> RISCV64_SATP_MODE_SHIFT;
  if (mode == 0ULL) {
    mode = RISCV64_SATP_MODE_SV39;
  }

  root_ppn = ((BOOT_U64)(BOOT_UPTR)&g_root[0]) >> 12;
  new_satp = (mode << RISCV64_SATP_MODE_SHIFT) | (root_ppn & RISCV64_SATP_PPN_MASK);

  write_satp(new_satp);
  sfence_vma_all();

  result->old_satp = old_satp;
  result->new_satp = new_satp;
  result->identity_bytes_mapped = RISCV64_IDENTITY_GIB * RISCV64_GIB;
  return 1;
}
