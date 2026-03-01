#include "memory_init.h"

void arch_memory_init(boot_info_t *boot_info) {
  boot_info_ext_sparc64_t *sparc_ext;

  if (boot_info == (boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_SPARC64) {
    return;
  }

  if ((boot_info->valid_mask & BOOT_INFO_HAS_ARCH_DATA) == 0 || boot_info->arch_data_ptr == 0 ||
      boot_info->arch_data_size < (BOOT_U64)sizeof(boot_info_ext_sparc64_t)) {
    return;
  }

  sparc_ext = (boot_info_ext_sparc64_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
  sparc_ext->mem_init_status = BOOT_MEM_INIT_STATUS_DEFERRED;
  sparc_ext->mem_old_root = boot_info->vm_root_table;
  sparc_ext->mem_new_root = boot_info->vm_root_table;
  sparc_ext->mem_mapped_bytes = 0;
}
