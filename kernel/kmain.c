#include "kernel.h"

#ifndef CORE_ARCH_NAME
#define CORE_ARCH_NAME "unknown"
#endif

static const char *boot_info_arch_name(BOOT_U64 arch_id) {
  switch (arch_id) {
  case BOOT_INFO_ARCH_X86_64:
    return "x86_64";
  case BOOT_INFO_ARCH_ARM64:
    return "arm64";
  case BOOT_INFO_ARCH_RISCV64:
    return "riscv64";
  case BOOT_INFO_ARCH_MIPS:
    return "mips";
  case BOOT_INFO_ARCH_SPARC64:
    return "sparc64";
  default:
    return "unknown";
  }
}

void kmain(const boot_info_t *boot_info) {
  arch_puts("HELLO FROM CORE KERNEL (" CORE_ARCH_NAME ") We good!\n");
  if (boot_info == (const boot_info_t *)0) {
    arch_puts("boot_info: null\n");
  } else {
    kprintf("boot_info.abi_version=0x%llx\n", boot_info->abi_version);
    kprintf("boot_info.arch_id=%u (%s)\n", (unsigned)boot_info->arch_id,
            boot_info_arch_name(boot_info->arch_id));
    kprintf("boot_info.valid_mask=0x%llx\n", boot_info->valid_mask);
    kprintf("boot_info.entry_pc=0x%llx\n", boot_info->entry_pc);
    kprintf("boot_info.entry_sp=0x%llx\n", boot_info->entry_sp);
    kprintf("boot_info.vm_enabled=%u\n", (unsigned)boot_info->vm_enabled);
    kprintf("boot_info.vm_root_table=0x%llx\n", boot_info->vm_root_table);
    kprintf("boot_info.uefi_system_table=0x%llx\n", boot_info->uefi_system_table);
    kprintf("boot_info.uefi_configuration_table=0x%llx\n", boot_info->uefi_configuration_table);
    kprintf("boot_info.acpi_rsdp=0x%llx\n", boot_info->acpi_rsdp);
    kprintf("boot_info.dtb_ptr=0x%llx\n", boot_info->dtb_ptr);
    kprintf("boot_info.boot_cpu_id=0x%llx\n", boot_info->boot_cpu_id);
    kprintf("boot_info.arch_data_ptr=0x%llx size=0x%llx\n", boot_info->arch_data_ptr,
            boot_info->arch_data_size);
    kprintf("boot_info.memory_map=0x%llx size=0x%llx desc_size=0x%llx desc_ver=0x%llx\n",
            boot_info->memory_map, boot_info->memory_map_size,
            boot_info->memory_map_descriptor_size, boot_info->memory_map_descriptor_version);
    kprintf("boot_info.framebuffer=0x%llx %ux%u ppsl=%u fmt=%u\n",
            boot_info->framebuffer_base, boot_info->framebuffer_width,
            boot_info->framebuffer_height, boot_info->framebuffer_pixels_per_scanline,
            boot_info->framebuffer_format);
    kprintf("boot_info.serial_port=0x%llx\n", boot_info->serial_port);

    if ((boot_info->valid_mask & BOOT_INFO_HAS_ARCH_DATA) != 0 && boot_info->arch_data_ptr != 0) {
      if (boot_info->arch_id == BOOT_INFO_ARCH_X86_64 || boot_info->arch_id == BOOT_INFO_ARCH_ARM64) {
        const boot_info_ext_uefi_t *uefi_ext =
            (const boot_info_ext_uefi_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
        kprintf("uefi_ext.image_handle=0x%llx\n", uefi_ext->image_handle);
        kprintf("uefi_ext.boot_services=0x%llx runtime_services=0x%llx\n",
                uefi_ext->boot_services, uefi_ext->runtime_services);
        kprintf("uefi_ext.con_out=0x%llx std_err=0x%llx\n", uefi_ext->con_out, uefi_ext->std_err);
      } else if (boot_info->arch_id == BOOT_INFO_ARCH_RISCV64) {
        const boot_info_ext_riscv64_t *riscv_ext =
            (const boot_info_ext_riscv64_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
        kprintf("riscv_ext.hart_id=0x%llx dtb_ptr=0x%llx\n", riscv_ext->hart_id, riscv_ext->dtb_ptr);
        kprintf("riscv_ext.satp=0x%llx uart=0x%llx ram=[0x%llx..0x%llx)\n", riscv_ext->satp,
                riscv_ext->uart_base, riscv_ext->ram_base, riscv_ext->ram_base + riscv_ext->ram_size);
      } else if (boot_info->arch_id == BOOT_INFO_ARCH_MIPS) {
        const boot_info_ext_mips_t *mips_ext =
            (const boot_info_ext_mips_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
        kprintf("mips_ext.a0..a3=[0x%llx 0x%llx 0x%llx 0x%llx]\n", mips_ext->entry_a0, mips_ext->entry_a1,
                mips_ext->entry_a2, mips_ext->entry_a3);
        kprintf("mips_ext.uart=0x%llx ram=[0x%llx..0x%llx)\n", mips_ext->uart_base, mips_ext->ram_base,
                mips_ext->ram_base + mips_ext->ram_size);
      } else if (boot_info->arch_id == BOOT_INFO_ARCH_SPARC64) {
        const boot_info_ext_sparc64_t *sparc_ext =
            (const boot_info_ext_sparc64_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
        kprintf("sparc_ext.prom_o0=0x%llx prom_o1=0x%llx prom_g7=0x%llx\n", sparc_ext->prom_o0,
                sparc_ext->prom_o1, sparc_ext->prom_g7);
        kprintf("sparc_ext.uart=0x%llx ram=[0x%llx..0x%llx)\n", sparc_ext->uart_base, sparc_ext->ram_base,
                sparc_ext->ram_base + sparc_ext->ram_size);
      }
    }
  }

  for (;;) {
    arch_halt();
  }
}
