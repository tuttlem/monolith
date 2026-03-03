# 135 IOMMU Subsystem

## Goal

Introduce an optional but standardized IOMMU layer for secure DMA isolation without imposing policy on OS design.

## In Scope

- IOMMU domain abstraction.
- Attach/detach device to domain.
- Map/unmap IOVA ranges.
- Fault reporting hook.
- Passthrough mode support.

## Out of Scope

- Mandatory use for all devices.
- Security policy decisions (left to OS).

## Public Interfaces

- Header: `kernel/include/iommu.h`
- APIs:
  - `status_t iommu_init(const boot_info_t *boot_info)`
  - `status_t iommu_domain_create(iommu_domain_t *out)`
  - `status_t iommu_attach(iommu_domain_t dom, device_id_t dev)`
  - `status_t iommu_map(iommu_domain_t dom, iova_t iova, phys_addr_t pa, BOOT_U64 len, iommu_perm_t perm)`
  - `status_t iommu_unmap(...)`

## Architecture Backends

- `x86_64`: VT-d/AMD-Vi capable path with passthrough fallback.
- `arm64`: SMMU capable path with passthrough fallback.
- `riscv64`: IOMMU extension path where present; otherwise deferred cleanly.

## Tests

- Domain create/attach/map/unmap unit tests.
- DMA path integration with and without iommu enabled.

## Acceptance Criteria

1. DMA API can route through iommu when available.
2. Feature-gated fallback remains functional without iommu hardware.
3. Fault events can be surfaced to diagnostics.
