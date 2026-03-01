#include "kernel.h"

extern BOOT_U64 __prom_o0;
extern BOOT_U64 __prom_o1;
extern BOOT_U64 __prom_o2;
extern BOOT_U64 __prom_o3;
extern BOOT_U64 __prom_o4;
extern BOOT_U64 __prom_o5;
extern BOOT_U64 __prom_g7;

#define SPARC64_UART_BASE 0x1fe020003f8ULL
#define SPARC64_RAM_BASE 0x0000000000000000ULL
#define SPARC64_RAM_SIZE (256ULL * 1024ULL * 1024ULL)

static BOOT_U64 read_sp(void) {
  BOOT_U64 sp;
  __asm__ volatile("mov %%sp, %0" : "=r"(sp));
  return sp;
}

static BOOT_U64 read_pc_hint(void) {
  BOOT_U64 o7;
  __asm__ volatile("mov %%o7, %0" : "=r"(o7));
  return o7;
}

void arch_main(void) {
  boot_info_t boot_info;
  boot_info_ext_sparc64_t sparc_ext;

  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.arch_id = BOOT_INFO_ARCH_SPARC64;
  boot_info.valid_mask = BOOT_INFO_HAS_ENTRY_STATE | BOOT_INFO_HAS_ARCH_DATA | BOOT_INFO_HAS_SERIAL;
  boot_info.entry_pc = read_pc_hint();
  boot_info.entry_sp = read_sp();
  boot_info.vm_enabled = 0;
  boot_info.vm_root_table = 0;
  boot_info.uefi_system_table = 0;
  boot_info.uefi_configuration_table = 0;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  boot_info.memory_region_count = 1;
  boot_info.memory_region_capacity = BOOT_INFO_MAX_MEM_REGIONS;
  boot_info.memory_regions[0].base = SPARC64_RAM_BASE;
  boot_info.memory_regions[0].size = SPARC64_RAM_SIZE;
  boot_info.memory_regions[0].kind = BOOT_MEM_REGION_USABLE;
  boot_info.memory_regions[0].reserved = 0;
  boot_info.valid_mask |= BOOT_INFO_HAS_MEM_REGIONS;
  boot_info.acpi_rsdp = 0;
  boot_info.dtb_ptr = 0;
  boot_info.boot_cpu_id = 0;
  sparc_ext.prom_o0 = __prom_o0;
  sparc_ext.prom_o1 = __prom_o1;
  sparc_ext.prom_o2 = __prom_o2;
  sparc_ext.prom_o3 = __prom_o3;
  sparc_ext.prom_o4 = __prom_o4;
  sparc_ext.prom_o5 = __prom_o5;
  sparc_ext.prom_g7 = __prom_g7;
  sparc_ext.uart_base = SPARC64_UART_BASE;
  sparc_ext.ram_base = SPARC64_RAM_BASE;
  sparc_ext.ram_size = SPARC64_RAM_SIZE;
  sparc_ext.mem_init_status = BOOT_MEM_INIT_STATUS_NONE;
  sparc_ext.mem_old_root = 0;
  sparc_ext.mem_new_root = 0;
  sparc_ext.mem_mapped_bytes = 0;
  boot_info.arch_data_ptr = (BOOT_U64)&sparc_ext;
  boot_info.arch_data_size = (BOOT_U64)sizeof(sparc_ext);
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  boot_info.serial_port = SPARC64_UART_BASE;

  kmain(&boot_info);
}
