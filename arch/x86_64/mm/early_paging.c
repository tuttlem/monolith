#include "arch/x86_64/early_paging.h"

#define X64_PAGE_PRESENT (1ULL << 0)
#define X64_PAGE_WRITABLE (1ULL << 1)
#define X64_PAGE_LARGE (1ULL << 7)

#define X64_PAGE_SIZE_2M (2ULL * 1024ULL * 1024ULL)
#define X64_IDENTITY_GIB 4ULL
#define X64_PDPT_ENTRIES ((X64_IDENTITY_GIB))

static BOOT_U64 g_pml4[512] __attribute__((aligned(4096)));
static BOOT_U64 g_pdpt[512] __attribute__((aligned(4096)));
static BOOT_U64 g_pd[X64_PDPT_ENTRIES][512] __attribute__((aligned(4096)));

static BOOT_U64 read_cr3(void) {
  BOOT_U64 cr3;
  __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
  return cr3;
}

static void write_cr3(BOOT_U64 cr3) { __asm__ volatile("movq %0, %%cr3" : : "r"(cr3) : "memory"); }

static void zero_u64(BOOT_U64 *p, BOOT_U64 count) {
  BOOT_U64 i;
  for (i = 0; i < count; ++i) {
    p[i] = 0;
  }
}

int x86_64_early_paging_takeover(x86_64_early_paging_result_t *result) {
  BOOT_U64 i;
  BOOT_U64 j;
  BOOT_U64 page_index = 0;
  BOOT_U64 old_cr3;
  BOOT_U64 new_cr3;

  if (result == (x86_64_early_paging_result_t *)0) {
    return 0;
  }

  zero_u64(g_pml4, 512);
  zero_u64(g_pdpt, 512);
  zero_u64(&g_pd[0][0], X64_PDPT_ENTRIES * 512ULL);

  g_pml4[0] = ((BOOT_U64)(BOOT_UPTR)&g_pdpt[0]) | X64_PAGE_PRESENT | X64_PAGE_WRITABLE;

  for (i = 0; i < X64_PDPT_ENTRIES; ++i) {
    g_pdpt[i] = ((BOOT_U64)(BOOT_UPTR)&g_pd[i][0]) | X64_PAGE_PRESENT | X64_PAGE_WRITABLE;
    for (j = 0; j < 512ULL; ++j) {
      BOOT_U64 phys = page_index * X64_PAGE_SIZE_2M;
      g_pd[i][j] = phys | X64_PAGE_PRESENT | X64_PAGE_WRITABLE | X64_PAGE_LARGE;
      ++page_index;
    }
  }

  old_cr3 = read_cr3();
  new_cr3 = (BOOT_U64)(BOOT_UPTR)&g_pml4[0];
  write_cr3(new_cr3);

  result->old_cr3 = old_cr3;
  result->new_cr3 = new_cr3;
  result->identity_bytes_mapped = X64_IDENTITY_GIB * 1024ULL * 1024ULL * 1024ULL;
  return 1;
}
