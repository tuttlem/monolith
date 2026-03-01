#ifndef KERNEL_BOOT_INFO_H
#define KERNEL_BOOT_INFO_H

typedef unsigned long long BOOT_U64;
typedef unsigned int BOOT_U32;
#if __SIZEOF_POINTER__ == 8
typedef unsigned long long BOOT_UPTR;
#else
typedef unsigned long BOOT_UPTR;
#endif

#define BOOT_INFO_ABI_VERSION 2ULL

#define BOOT_INFO_ARCH_UNKNOWN 0ULL
#define BOOT_INFO_ARCH_X86_64 1ULL
#define BOOT_INFO_ARCH_ARM64 2ULL
#define BOOT_INFO_ARCH_RISCV64 3ULL
#define BOOT_INFO_ARCH_MIPS 4ULL
#define BOOT_INFO_ARCH_SPARC64 5ULL

#define BOOT_INFO_HAS_ENTRY_STATE (1ULL << 0)
#define BOOT_INFO_HAS_VM_STATE (1ULL << 1)
#define BOOT_INFO_HAS_UEFI_SYSTEM_TABLE (1ULL << 2)
#define BOOT_INFO_HAS_UEFI_CONFIG_TABLE (1ULL << 3)
#define BOOT_INFO_HAS_MEMMAP (1ULL << 4)
#define BOOT_INFO_HAS_ACPI_RSDP (1ULL << 5)
#define BOOT_INFO_HAS_FRAMEBUFFER (1ULL << 6)
#define BOOT_INFO_HAS_SERIAL (1ULL << 7)
#define BOOT_INFO_HAS_DTB (1ULL << 8)
#define BOOT_INFO_HAS_BOOT_CPU_ID (1ULL << 9)
#define BOOT_INFO_HAS_ARCH_DATA (1ULL << 10)
#define BOOT_INFO_HAS_MEM_REGIONS (1ULL << 11)

#define BOOT_INFO_MAX_MEM_REGIONS 64U

#define BOOT_MEM_INIT_STATUS_NONE 0ULL
#define BOOT_MEM_INIT_STATUS_DONE 1ULL
#define BOOT_MEM_INIT_STATUS_DEFERRED 2ULL
#define BOOT_MEM_INIT_STATUS_FAILED 3ULL

#define BOOT_MEM_REGION_USABLE 1U
#define BOOT_MEM_REGION_RESERVED 2U
#define BOOT_MEM_REGION_ACPI_RECLAIM 3U
#define BOOT_MEM_REGION_ACPI_NVS 4U
#define BOOT_MEM_REGION_MMIO 5U

typedef struct {
  BOOT_U64 base;
  BOOT_U64 size;
  BOOT_U32 kind;
  BOOT_U32 reserved;
} boot_mem_region_t;

struct boot_info {
  /* ABI identity/versioning */
  BOOT_U64 abi_version;
  BOOT_U64 arch_id;
  BOOT_U64 valid_mask;

  /* CPU entry snapshot at boot -> kernel handoff */
  BOOT_U64 entry_pc;
  BOOT_U64 entry_sp;
  BOOT_U64 vm_enabled;
  BOOT_U64 vm_root_table;

  /* Firmware pointers */
  BOOT_U64 uefi_system_table;
  BOOT_U64 uefi_configuration_table;

  /* Memory map (UEFI memory descriptor stream) */
  BOOT_U64 memory_map;
  BOOT_U64 memory_map_size;
  BOOT_U64 memory_map_descriptor_size;
  BOOT_U64 memory_map_descriptor_version;
  BOOT_U32 memory_region_count;
  BOOT_U32 memory_region_capacity;
  boot_mem_region_t memory_regions[BOOT_INFO_MAX_MEM_REGIONS];

  /* Platform tables */
  BOOT_U64 acpi_rsdp;
  BOOT_U64 dtb_ptr;
  BOOT_U64 boot_cpu_id;
  BOOT_U64 arch_data_ptr;
  BOOT_U64 arch_data_size;

  /* Early console outputs */
  BOOT_U64 framebuffer_base;
  BOOT_U32 framebuffer_width;
  BOOT_U32 framebuffer_height;
  BOOT_U32 framebuffer_pixels_per_scanline;
  BOOT_U32 framebuffer_format;
  BOOT_U64 serial_port;
};

typedef struct boot_info boot_info_t;

typedef struct {
  BOOT_U64 image_handle;
  BOOT_U64 system_table;
  BOOT_U64 configuration_table;
  BOOT_U64 boot_services;
  BOOT_U64 runtime_services;
  BOOT_U64 con_out;
  BOOT_U64 std_err;
  BOOT_U64 firmware_vendor;
  BOOT_U64 firmware_revision;
  BOOT_U64 mem_init_status;
  BOOT_U64 mem_old_root;
  BOOT_U64 mem_new_root;
  BOOT_U64 mem_mapped_bytes;
  BOOT_U64 paging_old_cr3;
  BOOT_U64 paging_new_cr3;
  BOOT_U64 paging_identity_bytes;
} boot_info_ext_uefi_t;

typedef struct {
  BOOT_U64 hart_id;
  BOOT_U64 dtb_ptr;
  BOOT_U64 satp;
  BOOT_U64 uart_base;
  BOOT_U64 ram_base;
  BOOT_U64 ram_size;
  BOOT_U64 entry_a0;
  BOOT_U64 entry_a1;
  BOOT_U64 mem_init_status;
  BOOT_U64 mem_old_root;
  BOOT_U64 mem_new_root;
  BOOT_U64 mem_mapped_bytes;
} boot_info_ext_riscv64_t;

typedef struct {
  BOOT_U64 entry_a0;
  BOOT_U64 entry_a1;
  BOOT_U64 entry_a2;
  BOOT_U64 entry_a3;
  BOOT_U64 uart_base;
  BOOT_U64 ram_base;
  BOOT_U64 ram_size;
  BOOT_U64 mem_init_status;
  BOOT_U64 mem_old_root;
  BOOT_U64 mem_new_root;
  BOOT_U64 mem_mapped_bytes;
} boot_info_ext_mips_t;

typedef struct {
  BOOT_U64 prom_o0;
  BOOT_U64 prom_o1;
  BOOT_U64 prom_o2;
  BOOT_U64 prom_o3;
  BOOT_U64 prom_o4;
  BOOT_U64 prom_o5;
  BOOT_U64 prom_g7;
  BOOT_U64 uart_base;
  BOOT_U64 ram_base;
  BOOT_U64 ram_size;
  BOOT_U64 mem_init_status;
  BOOT_U64 mem_old_root;
  BOOT_U64 mem_new_root;
  BOOT_U64 mem_mapped_bytes;
} boot_info_ext_sparc64_t;

#endif
