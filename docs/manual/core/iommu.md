# IOMMU Subsystem

This layer defines a generic IOMMU interface for optional DMA isolation and IOVA mapping.

## Purpose

- Keep IOMMU hardware specifics out of generic DMA/device code.
- Provide stable domain/attach/map APIs for future backend upgrades.
- Support passthrough as an explicit mode.

## Public API

Header: `kernel/include/iommu.h`

- `status_t iommu_init(const boot_info_t *boot_info)`
- `status_t iommu_domain_create(iommu_domain_t *out_domain)`
- `status_t iommu_attach(iommu_domain_t domain, BOOT_U64 device_id)`
- `status_t iommu_detach(iommu_domain_t domain, BOOT_U64 device_id)`
- `status_t iommu_map(iommu_domain_t domain, iova_t iova, phys_addr_t pa, BOOT_U64 len, iommu_perm_t perm)`
- `status_t iommu_unmap(iommu_domain_t domain, iova_t iova, BOOT_U64 len)`
- `status_t iommu_set_passthrough(iommu_domain_t domain, int enabled)`

## Notes

- Baseline implementation now provides active domain/map bookkeeping (`iommu_init`, `iommu_map`, `iommu_unmap` return concrete statuses), which allows OS layers to rely on stable map lifecycle semantics even before hardware-specific remapper programming is added.
