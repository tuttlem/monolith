#include "memory_init.h"
#include "arch/x86_64/early_paging.h"

status_t arch_memory_init(boot_info_t *boot_info) {
  x86_64_early_paging_result_t paging_result;
  boot_info_ext_uefi_t *uefi_ext = (boot_info_ext_uefi_t *)0;

  if (boot_info == (boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }

  if ((boot_info->valid_mask & BOOT_INFO_HAS_ARCH_DATA) != 0 && boot_info->arch_data_ptr != 0 &&
      boot_info->arch_data_size >= (BOOT_U64)sizeof(boot_info_ext_uefi_t)) {
    uefi_ext = (boot_info_ext_uefi_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
    uefi_ext->mem_init_status = BOOT_MEM_INIT_STATUS_NONE;
    uefi_ext->mem_old_root = boot_info->vm_root_table;
    uefi_ext->mem_new_root = boot_info->vm_root_table;
    uefi_ext->mem_mapped_bytes = 0;
  }

  paging_result.old_cr3 = boot_info->vm_root_table;
  paging_result.new_cr3 = boot_info->vm_root_table;
  paging_result.identity_bytes_mapped = 0;
  if (!x86_64_early_paging_takeover(&paging_result)) {
    if (uefi_ext != (boot_info_ext_uefi_t *)0) {
      uefi_ext->mem_init_status = BOOT_MEM_INIT_STATUS_FAILED;
    }
    return STATUS_INTERNAL;
  }

  boot_info->vm_enabled = 1;
  boot_info->vm_root_table = paging_result.new_cr3;
  if (uefi_ext != (boot_info_ext_uefi_t *)0) {
    uefi_ext->mem_init_status = BOOT_MEM_INIT_STATUS_DONE;
    uefi_ext->mem_old_root = paging_result.old_cr3;
    uefi_ext->mem_new_root = paging_result.new_cr3;
    uefi_ext->mem_mapped_bytes = paging_result.identity_bytes_mapped;
    uefi_ext->paging_old_cr3 = paging_result.old_cr3;
    uefi_ext->paging_new_cr3 = paging_result.new_cr3;
    uefi_ext->paging_identity_bytes = paging_result.identity_bytes_mapped;
  }
  return STATUS_OK;
}
