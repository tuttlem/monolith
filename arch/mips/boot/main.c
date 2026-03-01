#include "kernel.h"

#ifndef UART_BASE
#define UART_BASE 0xBFD003F8UL
#endif

#define MIPS_RAM_BASE 0x00000000ULL
#define MIPS_RAM_SIZE (256ULL * 1024ULL * 1024ULL)

static BOOT_U64 read_sp(void) {
  BOOT_U64 sp;
  __asm__ volatile("move %0, $sp" : "=r"(sp));
  return sp;
}

static BOOT_U64 read_pc(void) {
  BOOT_U64 ra;
  __asm__ volatile("move %0, $ra" : "=r"(ra));
  return ra;
}

void arch_main(BOOT_U64 entry_a0, BOOT_U64 entry_a1, BOOT_U64 entry_a2, BOOT_U64 entry_a3) {
  boot_info_t boot_info;
  boot_info_ext_mips_t mips_ext;

  boot_info.abi_version = BOOT_INFO_ABI_VERSION;
  boot_info.arch_id = BOOT_INFO_ARCH_MIPS;
  boot_info.valid_mask = BOOT_INFO_HAS_ENTRY_STATE | BOOT_INFO_HAS_ARCH_DATA | BOOT_INFO_HAS_SERIAL;
  boot_info.entry_pc = read_pc();
  boot_info.entry_sp = read_sp();
  boot_info.vm_enabled = 0;
  boot_info.vm_root_table = 0;
  boot_info.uefi_system_table = 0;
  boot_info.uefi_configuration_table = 0;
  boot_info.memory_map = 0;
  boot_info.memory_map_size = 0;
  boot_info.memory_map_descriptor_size = 0;
  boot_info.memory_map_descriptor_version = 0;
  boot_info.acpi_rsdp = 0;
  boot_info.dtb_ptr = 0;
  boot_info.boot_cpu_id = 0;
  mips_ext.entry_a0 = entry_a0;
  mips_ext.entry_a1 = entry_a1;
  mips_ext.entry_a2 = entry_a2;
  mips_ext.entry_a3 = entry_a3;
  mips_ext.uart_base = (BOOT_U64)UART_BASE;
  mips_ext.ram_base = MIPS_RAM_BASE;
  mips_ext.ram_size = MIPS_RAM_SIZE;
  boot_info.arch_data_ptr = (BOOT_U64)&mips_ext;
  boot_info.arch_data_size = (BOOT_U64)sizeof(mips_ext);
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  boot_info.serial_port = (BOOT_U64)UART_BASE;

  kmain(&boot_info);
}
