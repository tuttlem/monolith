#include "kernel.h"

#define RISCV64_UART_BASE 0x10000000ULL
#define RISCV64_RAM_BASE 0x80000000ULL
#define RISCV64_RAM_SIZE (512ULL * 1024ULL * 1024ULL)

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

void arch_main(BOOT_U64 hart_id, BOOT_U64 dtb_ptr) {
  boot_info_t boot_info;
  boot_info_ext_riscv64_t riscv_ext;
  BOOT_U64 satp;

  satp = read_satp();
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
  riscv_ext.ram_base = RISCV64_RAM_BASE;
  riscv_ext.ram_size = RISCV64_RAM_SIZE;
  riscv_ext.entry_a0 = hart_id;
  riscv_ext.entry_a1 = dtb_ptr;
  boot_info.arch_data_ptr = (BOOT_U64)&riscv_ext;
  boot_info.arch_data_size = (BOOT_U64)sizeof(riscv_ext);
  boot_info.framebuffer_base = 0;
  boot_info.framebuffer_width = 0;
  boot_info.framebuffer_height = 0;
  boot_info.framebuffer_pixels_per_scanline = 0;
  boot_info.framebuffer_format = 0;
  kmain(&boot_info);
}
