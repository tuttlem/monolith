#include "memory_init.h"
#include "arch/mips/early_paging.h"

static BOOT_U64 usable_bytes(const boot_info_t *boot_info) {
  BOOT_U64 total = 0;
  BOOT_U32 i;

  for (i = 0; i < boot_info->memory_region_count; ++i) {
    const boot_mem_region_t *region = &boot_info->memory_regions[i];
    if (region->kind == BOOT_MEM_REGION_USABLE) {
      total += region->size;
    }
  }
  return total;
}

status_t arch_memory_init(boot_info_t *boot_info) {
  boot_info_ext_mips_t *mips_ext;
  mips_early_paging_result_t paging_result;

  if (boot_info == (boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_MIPS) {
    return STATUS_INVALID_ARG;
  }

  if ((boot_info->valid_mask & BOOT_INFO_HAS_ARCH_DATA) == 0 || boot_info->arch_data_ptr == 0 ||
      boot_info->arch_data_size < (BOOT_U64)sizeof(boot_info_ext_mips_t)) {
    return STATUS_INTERNAL;
  }

  mips_ext = (boot_info_ext_mips_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
  mips_ext->mem_init_status = BOOT_MEM_INIT_STATUS_NONE;
  mips_ext->mem_old_root = boot_info->vm_root_table;
  mips_ext->mem_new_root = boot_info->vm_root_table;
  mips_ext->mem_mapped_bytes = 0;

  if (!mips_early_paging_takeover(&paging_result)) {
    mips_ext->mem_init_status = BOOT_MEM_INIT_STATUS_FAILED;
    return STATUS_INTERNAL;
  }

  paging_result.old_root = boot_info->vm_root_table;
  boot_info->vm_root_table = paging_result.new_root;

  mips_ext->mem_init_status = BOOT_MEM_INIT_STATUS_DONE;
  mips_ext->mem_old_root = paging_result.old_root;
  mips_ext->mem_new_root = paging_result.new_root;
  mips_ext->mem_mapped_bytes = usable_bytes(boot_info);
  return STATUS_OK;
}
