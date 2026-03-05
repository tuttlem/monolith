#include "memory_init.h"
#include "arch/arm64/early_paging.h"

static u64 usable_bytes(const boot_info_t *boot_info) {
  u64 total = 0;
  u32 i;

  for (i = 0; i < boot_info->memory_region_count; ++i) {
    const boot_mem_region_t *region = &boot_info->memory_regions[i];
    if (region->kind == BOOT_MEM_REGION_USABLE) {
      total += region->size;
    }
  }
  return total;
}

status_t arch_memory_init(boot_info_t *boot_info) {
  boot_info_ext_uefi_t *uefi_ext;
  arm64_early_paging_result_t paging_result;
  u64 mapped;

  if (boot_info == (boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }

  mapped = usable_bytes(boot_info);

  if ((boot_info->valid_mask & BOOT_INFO_HAS_ARCH_DATA) == 0 || boot_info->arch_data_ptr == 0 ||
      boot_info->arch_data_size < (u64)sizeof(boot_info_ext_uefi_t)) {
    return STATUS_INTERNAL;
  }

  uefi_ext = (boot_info_ext_uefi_t *)(uptr)boot_info->arch_data_ptr;
  uefi_ext->mem_init_status = BOOT_MEM_INIT_STATUS_NONE;
  uefi_ext->mem_old_root = boot_info->vm_root_table;
  uefi_ext->mem_new_root = boot_info->vm_root_table;
  uefi_ext->mem_mapped_bytes = 0;
  uefi_ext->paging_old_cr3 = boot_info->vm_root_table;
  uefi_ext->paging_new_cr3 = boot_info->vm_root_table;
  uefi_ext->paging_identity_bytes = 0;

  if (!arm64_early_paging_takeover(boot_info, &paging_result)) {
    uefi_ext->mem_init_status = BOOT_MEM_INIT_STATUS_FAILED;
    return STATUS_INTERNAL;
  }

  boot_info->vm_enabled = 1;
  boot_info->vm_root_table = paging_result.new_ttbr0;

  uefi_ext->mem_init_status = BOOT_MEM_INIT_STATUS_DONE;
  uefi_ext->mem_old_root = paging_result.old_ttbr0;
  uefi_ext->mem_new_root = paging_result.new_ttbr0;
  uefi_ext->mem_mapped_bytes = mapped;
  uefi_ext->paging_old_cr3 = paging_result.old_ttbr0;
  uefi_ext->paging_new_cr3 = paging_result.new_ttbr0;
  uefi_ext->paging_identity_bytes = paging_result.identity_bytes_mapped;
  return STATUS_OK;
}
