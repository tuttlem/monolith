#include "kernel.h"

typedef unsigned char u8;

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

#define RISCV64_UART_BASE 0x10000000ULL
#define RISCV64_RAM_BASE_FALLBACK 0x80000000ULL
#define RISCV64_RAM_SIZE_FALLBACK (512ULL * 1024ULL * 1024ULL)

#define FDT_MAGIC 0xd00dfeedU
#define FDT_BEGIN_NODE 1U
#define FDT_END_NODE 2U
#define FDT_PROP 3U
#define FDT_NOP 4U
#define FDT_END 9U

static BOOT_U64 read_sp(void) {
  BOOT_U64 sp;
  __asm__ volatile("mv %0, sp" : "=r"(sp));
  return sp;
}

static BOOT_U64 read_pc(void) {
  BOOT_U64 pc;
  __asm__ volatile("auipc %0, 0" : "=r"(pc));
  return pc;
}

static BOOT_U64 read_satp(void) {
  BOOT_U64 satp;
  __asm__ volatile("csrr %0, satp" : "=r"(satp));
  return satp;
}

static BOOT_U32 be32_at(const u8 *p) {
  return ((BOOT_U32)p[0] << 24) | ((BOOT_U32)p[1] << 16) | ((BOOT_U32)p[2] << 8) | (BOOT_U32)p[3];
}

static BOOT_U64 be64_at(const u8 *p) {
  BOOT_U64 hi = (BOOT_U64)be32_at(p);
  BOOT_U64 lo = (BOOT_U64)be32_at(p + 4);
  return (hi << 32) | lo;
}

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

static BOOT_U32 align4(BOOT_U32 v) { return (v + 3U) & ~3U; }

static int parse_fdt_memory(const void *dtb, BOOT_U64 *out_base, BOOT_U64 *out_size) {
  const fdt_header_t *hdr;
  const u8 *blob = (const u8 *)dtb;
  const u8 *struct_base;
  const u8 *struct_end;
  const char *strings_base;
  BOOT_U32 off_struct;
  BOOT_U32 size_struct;
  BOOT_U32 off_strings;
  BOOT_U32 depth = 0;
  BOOT_U32 memory_depth = 0;
  const u8 *p;

  if (dtb == (const void *)0 || out_base == (BOOT_U64 *)0 || out_size == (BOOT_U64 *)0) {
    return 0;
  }

  hdr = (const fdt_header_t *)dtb;
  if (be32_at((const u8 *)&hdr->magic) != FDT_MAGIC) {
    return 0;
  }

  off_struct = be32_at((const u8 *)&hdr->off_dt_struct);
  size_struct = be32_at((const u8 *)&hdr->size_dt_struct);
  off_strings = be32_at((const u8 *)&hdr->off_dt_strings);

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
        return 0;
      }
      ++p;
      p = struct_base + align4((BOOT_U32)(p - struct_base));
      ++depth;
      if (depth == 2U && (str_eq(name, "memory") || str_starts_with(name, "memory@"))) {
        memory_depth = depth;
      }
      continue;
    }

    if (token == FDT_END_NODE) {
      if (depth == memory_depth) {
        memory_depth = 0;
      }
      if (depth == 0) {
        return 0;
      }
      --depth;
      continue;
    }

    if (token == FDT_PROP) {
      BOOT_U32 len;
      BOOT_U32 nameoff;
      const char *prop_name;

      if (p + 8 > struct_end) {
        return 0;
      }

      len = be32_at(p);
      nameoff = be32_at(p + 4);
      p += 8;
      prop_name = strings_base + nameoff;

      if (memory_depth != 0 && depth == memory_depth && str_eq(prop_name, "reg")) {
        if (len >= 16U) {
          *out_base = be64_at(p);
          *out_size = be64_at(p + 8);
          return 1;
        }
        if (len >= 8U) {
          *out_base = (BOOT_U64)be32_at(p);
          *out_size = (BOOT_U64)be32_at(p + 4);
          return 1;
        }
      }

      p += align4(len);
      continue;
    }

    if (token == FDT_NOP) {
      continue;
    }

    if (token == FDT_END) {
      break;
    }

    return 0;
  }

  return 0;
}

static void add_memory_region(boot_info_t *boot_info, BOOT_U64 base, BOOT_U64 size, BOOT_U32 kind) {
  BOOT_U32 idx;

  if (size == 0 || boot_info->memory_region_count >= boot_info->memory_region_capacity) {
    return;
  }

  idx = boot_info->memory_region_count++;
  boot_info->memory_regions[idx].base = base;
  boot_info->memory_regions[idx].size = size;
  boot_info->memory_regions[idx].kind = kind;
  boot_info->memory_regions[idx].reserved = 0;
}

void arch_main(BOOT_U64 hart_id, BOOT_U64 dtb_ptr) {
  boot_info_t boot_info;
  boot_info_ext_riscv64_t riscv_ext;
  BOOT_U64 satp;
  BOOT_U64 ram_base = RISCV64_RAM_BASE_FALLBACK;
  BOOT_U64 ram_size = RISCV64_RAM_SIZE_FALLBACK;

  satp = read_satp();
  if (parse_fdt_memory((const void *)(BOOT_UPTR)dtb_ptr, &ram_base, &ram_size)) {
    /* DTB memory range overrides fallback values. */
  }

  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.arch_id = BOOT_INFO_ARCH_RISCV64;
  boot_info.valid_mask = BOOT_INFO_HAS_ENTRY_STATE | BOOT_INFO_HAS_ARCH_DATA;
  boot_info.entry_pc = read_pc();
  boot_info.entry_sp = read_sp();
  boot_info.vm_enabled = (satp != 0);
  boot_info.vm_root_table = satp;
  if (satp != 0) {
    boot_info.valid_mask |= BOOT_INFO_HAS_VM_STATE;
  }
  boot_info.uefi_system_table = 0;
  boot_info.uefi_configuration_table = 0;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  boot_info.memory_region_count = 0;
  boot_info.memory_region_capacity = BOOT_INFO_MAX_MEM_REGIONS;
  add_memory_region(&boot_info, ram_base, ram_size, BOOT_MEM_REGION_USABLE);
  if (boot_info.memory_region_count > 0) {
    boot_info.valid_mask |= BOOT_INFO_HAS_MEM_REGIONS;
  }
  boot_info.acpi_rsdp = 0;
  boot_info.dtb_ptr = dtb_ptr;
  boot_info.boot_cpu_id = hart_id;
  if (dtb_ptr != 0) {
    boot_info.valid_mask |= BOOT_INFO_HAS_DTB;
  }
  boot_info.valid_mask |= BOOT_INFO_HAS_BOOT_CPU_ID;
  boot_info.serial_port = RISCV64_UART_BASE;
  boot_info.valid_mask |= BOOT_INFO_HAS_SERIAL;
  riscv_ext.hart_id = hart_id;
  riscv_ext.dtb_ptr = dtb_ptr;
  riscv_ext.satp = satp;
  riscv_ext.uart_base = RISCV64_UART_BASE;
  riscv_ext.ram_base = ram_base;
  riscv_ext.ram_size = ram_size;
  riscv_ext.entry_a0 = hart_id;
  riscv_ext.entry_a1 = dtb_ptr;
  riscv_ext.mem_init_status = BOOT_MEM_INIT_STATUS_NONE;
  riscv_ext.mem_old_root = satp;
  riscv_ext.mem_new_root = satp;
  riscv_ext.mem_mapped_bytes = 0;
  boot_info.arch_data_ptr = (BOOT_U64)&riscv_ext;
  boot_info.arch_data_size = (BOOT_U64)sizeof(riscv_ext);
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  kmain(&boot_info);
}
