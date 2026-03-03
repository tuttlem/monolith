#include "arch_smp.h"
#include "arch_cpu.h"
#include "cpu_context.h"
#include "percpu.h"
#include "scheduler.h"
#include "smp.h"
#include "uefi.h"

static const EFI_GUID k_mp_services_guid = {
    0x3fdda605U, 0xa76eU, 0x4f46U, {0xadU, 0x29U, 0x12U, 0xf4U, 0x53U, 0x1bU, 0x3dU, 0x08U}};

typedef struct {
  EFI_MP_SERVICES_PROTOCOL *mp;
  volatile BOOT_U64 started;
} arm64_smp_context_t;

static void EFIAPI arm64_smp_ap_entry(VOID *arg) {
  arm64_smp_context_t *ctx = (arm64_smp_context_t *)arg;
  UINTN cpu_id = 0;

  if (ctx == (arm64_smp_context_t *)0 || ctx->mp == (EFI_MP_SERVICES_PROTOCOL *)0 || ctx->mp->WhoAmI == (EFI_MP_WHOAMI)0) {
    return;
  }
  if (EFI_ERROR(ctx->mp->WhoAmI(ctx->mp, &cpu_id))) {
    return;
  }

  if (percpu_register_current_cpu((BOOT_U64)cpu_id) == STATUS_OK) {
    __atomic_add_fetch(&ctx->started, 1ULL, __ATOMIC_SEQ_CST);
    smp_secondary_entry((BOOT_U64)cpu_id);
  }
}

status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus) {
  boot_info_ext_uefi_t *uefi_ext;
  EFI_BOOT_SERVICES *boot_services;
  EFI_MP_SERVICES_PROTOCOL *mp = (EFI_MP_SERVICES_PROTOCOL *)0;
  EFI_STATUS st;
  UINTN total = 1;
  UINTN enabled = 1;
  arm64_smp_context_t ctx;

  if (boot_info == (const boot_info_t *)0 || out_possible_cpus == (BOOT_U64 *)0 || out_started_cpus == (BOOT_U64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if (boot_info->arch_id != BOOT_INFO_ARCH_ARM64) {
    return STATUS_INVALID_ARG;
  }
  if ((boot_info->valid_mask & BOOT_INFO_HAS_ARCH_DATA) == 0 || boot_info->arch_data_ptr == 0 ||
      boot_info->arch_data_size < (BOOT_U64)sizeof(boot_info_ext_uefi_t)) {
    *out_possible_cpus = 1ULL;
    *out_started_cpus = 0ULL;
    return STATUS_OK;
  }

  uefi_ext = (boot_info_ext_uefi_t *)(BOOT_UPTR)boot_info->arch_data_ptr;
  boot_services = (EFI_BOOT_SERVICES *)(BOOT_UPTR)uefi_ext->boot_services;
  if (boot_services == (EFI_BOOT_SERVICES *)0 || boot_services->LocateProtocol == (EFI_LOCATE_PROTOCOL)0) {
    *out_possible_cpus = 1ULL;
    *out_started_cpus = 0ULL;
    return STATUS_OK;
  }

  st = boot_services->LocateProtocol((EFI_GUID *)&k_mp_services_guid, (VOID *)0, (VOID **)&mp);
  if (EFI_ERROR(st) || mp == (EFI_MP_SERVICES_PROTOCOL *)0 || mp->GetNumberOfProcessors == (EFI_MP_GET_NUMBER_OF_PROCESSORS)0 ||
      mp->StartupAllAPs == (EFI_MP_STARTUP_ALL_APS)0) {
    *out_possible_cpus = 1ULL;
    *out_started_cpus = 0ULL;
    return STATUS_OK;
  }

  st = mp->GetNumberOfProcessors(mp, &total, &enabled);
  if (EFI_ERROR(st)) {
    return STATUS_TRY_AGAIN;
  }

  *out_possible_cpus = (BOOT_U64)total;
  if (enabled <= 1U) {
    *out_started_cpus = 0ULL;
    return STATUS_OK;
  }

  ctx.mp = mp;
  ctx.started = 0;
  st = mp->StartupAllAPs(mp, arm64_smp_ap_entry, 0, (EFI_EVENT)0, 2000000U, &ctx, (UINTN **)0);
  *out_started_cpus = ctx.started;
  if (ctx.started != 0ULL) {
    return STATUS_OK;
  }
  if (EFI_ERROR(st)) {
    return STATUS_TRY_AGAIN;
  }

  return STATUS_OK;
}

status_t arch_context_switch(task_t *from, task_t *to) {
  if (from == (task_t *)0 || to == (task_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (from->arch_ctx == (void *)0 || to->arch_ctx == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  return cpu_context_switch((cpu_context_t *)from->arch_ctx, (cpu_context_t *)to->arch_ctx);
}

status_t arch_smp_cpu_start(BOOT_U64 cpu_id) {
  if (cpu_id == 0ULL) {
    return STATUS_OK;
  }
  return STATUS_NOT_SUPPORTED;
}

status_t arch_smp_ipi_send(BOOT_U64 cpu_id, BOOT_U64 kind) {
  (void)kind;
  if (cpu_id == arch_cpu_id()) {
    return STATUS_OK;
  }
  return STATUS_NOT_SUPPORTED;
}

status_t arch_smp_tlb_shootdown(BOOT_U64 mask, BOOT_U64 va, BOOT_U64 len) {
  (void)va;
  (void)len;
  if ((mask & (1ULL << arch_cpu_id())) != 0ULL) {
    arch_tlb_sync_local();
    if ((mask & ~(1ULL << arch_cpu_id())) == 0ULL) {
      return STATUS_OK;
    }
    return STATUS_NOT_SUPPORTED;
  }
  return STATUS_NOT_SUPPORTED;
}
