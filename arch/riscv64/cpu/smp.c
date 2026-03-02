#include "arch_smp.h"

#define FDT_MAGIC 0xd00dfeedU
#define FDT_BEGIN_NODE 1U
#define FDT_END_NODE 2U
#define FDT_PROP 3U
#define FDT_NOP 4U
#define FDT_END 9U

typedef struct {
  BOOT_U32 magic;
  BOOT_U32 totalsize;
  BOOT_U32 off_dt_struct;
  BOOT_U32 off_dt_strings;
  BOOT_U32 off_mem_rsvmap;
  BOOT_U32 version;
  BOOT_U32 last_comp_version;
  BOOT_U32 boot_cpuid_phys;
  BOOT_U32 size_dt_strings;
  BOOT_U32 size_dt_struct;
} fdt_header_t;

static BOOT_U32 be32_at(const unsigned char *p) {
  return ((BOOT_U32)p[0] << 24) | ((BOOT_U32)p[1] << 16) | ((BOOT_U32)p[2] << 8) | (BOOT_U32)p[3];
}

static BOOT_U32 align4(BOOT_U32 v) { return (v + 3U) & ~3U; }

static int str_eq(const char *a, const char *b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return (*a == '\0' && *b == '\0');
}

static int str_starts_with(const char *s, const char *prefix) {
  while (*prefix != '\0') {
    if (*s++ != *prefix++) {
      return 0;
    }
  }
  return 1;
}

static BOOT_U64 riscv64_dtb_cpu_count(const void *dtb) {
  const fdt_header_t *hdr;
  const unsigned char *blob = (const unsigned char *)dtb;
  const unsigned char *struct_base;
  const unsigned char *struct_end;
  const char *strings_base;
  const unsigned char *p;
  BOOT_U32 off_struct;
  BOOT_U32 size_struct;
  BOOT_U32 off_strings;
  BOOT_U32 depth = 0;
  BOOT_U64 count = 0;

  if (dtb == (const void *)0) {
    return 1ULL;
  }

  hdr = (const fdt_header_t *)dtb;
  if (be32_at((const unsigned char *)&hdr->magic) != FDT_MAGIC) {
    return 1ULL;
  }

  off_struct = be32_at((const unsigned char *)&hdr->off_dt_struct);
  size_struct = be32_at((const unsigned char *)&hdr->size_dt_struct);
  off_strings = be32_at((const unsigned char *)&hdr->off_dt_strings);
  struct_base = blob + off_struct;
  struct_end = struct_base + size_struct;
  strings_base = (const char *)(blob + off_strings);
  p = struct_base;

  while (p + 4 <= struct_end) {
    BOOT_U32 token = be32_at(p);
    p += 4;

    if (token == FDT_BEGIN_NODE) {
      const char *name = (const char *)p;
      while (p < struct_end && *p != '\0') {
        ++p;
      }
      if (p >= struct_end) {
        break;
      }
      ++p;
      p = struct_base + align4((BOOT_U32)(p - struct_base));
      ++depth;
      if (depth >= 2U && (str_eq(name, "cpu") || str_starts_with(name, "cpu@"))) {
        count += 1ULL;
      }
      continue;
    }

    if (token == FDT_END_NODE) {
      if (depth == 0U) {
        break;
      }
      --depth;
      continue;
    }

    if (token == FDT_PROP) {
      BOOT_U32 len;
      if (p + 8 > struct_end) {
        break;
      }
      len = be32_at(p);
      (void)strings_base;
      p += 8;
      p += align4(len);
      continue;
    }

    if (token == FDT_NOP) {
      continue;
    }
    if (token == FDT_END) {
      break;
    }
    break;
  }

  if (count == 0ULL) {
    count = 1ULL;
  }
  return count;
}

status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus) {
  if (boot_info == (const boot_info_t *)0 || out_possible_cpus == (BOOT_U64 *)0 || out_started_cpus == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_RISCV64) {
    return STATUS_INVALID_ARG;
  }

  *out_possible_cpus = riscv64_dtb_cpu_count((const void *)(BOOT_UPTR)boot_info->dtb_ptr);
  *out_started_cpus = 0ULL;
  return STATUS_DEFERRED;
}
