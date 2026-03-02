#include "arch/mips/early_paging.h"

#define MIPS_STAGE0_MAPPED_BYTES (256ULL * 1024ULL * 1024ULL)
#define MIPS_STAGE0_ENTRY_BYTES (4ULL * 1024ULL * 1024ULL)
#define MIPS_STAGE0_ENTRIES (MIPS_STAGE0_MAPPED_BYTES / MIPS_STAGE0_ENTRY_BYTES)

static BOOT_U64 g_stage0_root[1024] __attribute__((aligned(4096)));

static void zero_u64(BOOT_U64 *p, BOOT_U64 count) {
  BOOT_U64 i;
  for (i = 0; i < count; ++i) {
    p[i] = 0;
  }
}

int mips_early_paging_takeover(mips_early_paging_result_t *result) {
  BOOT_U64 i;

  if (result == (mips_early_paging_result_t *)0) {
    return 0;
  }

  zero_u64(g_stage0_root, 1024ULL);
  for (i = 0; i < MIPS_STAGE0_ENTRIES; ++i) {
    /* Stage-0 software page directory: entry i covers 4 MiB of identity space. */
    g_stage0_root[i] = i * MIPS_STAGE0_ENTRY_BYTES;
  }

  result->new_root = (BOOT_U64)(BOOT_UPTR)&g_stage0_root[0];
  result->identity_bytes_mapped = MIPS_STAGE0_MAPPED_BYTES;
  return 1;
}
