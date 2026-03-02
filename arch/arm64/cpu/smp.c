#include "arch_smp.h"

status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus) {
  if (boot_info == (const boot_info_t *)0 || out_possible_cpus == (BOOT_U64 *)0 || out_started_cpus == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }

  *out_possible_cpus = 1ULL;
  *out_started_cpus = 0ULL;
  return STATUS_DEFERRED;
}
