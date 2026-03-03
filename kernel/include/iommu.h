#ifndef KERNEL_IOMMU_H
#define KERNEL_IOMMU_H

#include "boot_info.h"
#include "status.h"

typedef BOOT_U64 iommu_domain_t;
typedef BOOT_U64 iova_t;
typedef BOOT_U64 phys_addr_t;

typedef enum {
  IOMMU_PERM_READ = 1U << 0,
  IOMMU_PERM_WRITE = 1U << 1,
  IOMMU_PERM_EXEC = 1U << 2
} iommu_perm_t;

status_t iommu_init(const boot_info_t *boot_info);
status_t iommu_domain_create(iommu_domain_t *out_domain);
status_t iommu_attach(iommu_domain_t domain, BOOT_U64 device_id);
status_t iommu_detach(iommu_domain_t domain, BOOT_U64 device_id);
status_t iommu_map(iommu_domain_t domain, iova_t iova, phys_addr_t pa, BOOT_U64 len, iommu_perm_t perm);
status_t iommu_unmap(iommu_domain_t domain, iova_t iova, BOOT_U64 len);
status_t iommu_set_passthrough(iommu_domain_t domain, int enabled);

#endif
