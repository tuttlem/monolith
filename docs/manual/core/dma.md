# DMA Mapping API

This layer provides a generic DMA contract for drivers and domain code.

## Purpose

- Standardize map/unmap and sync semantics.
- Keep DMA cache/coherency handling architecture-neutral.
- Allow device-specific constraints without exposing backend details.

## Public API

Header: `kernel/include/dma.h`

- `status_t dma_init(const boot_info_t *boot_info)`
- `status_t dma_map(u64 device_id, void *cpu_ptr, u64 len, dma_dir_t dir, dma_addr_t *out)`
- `status_t dma_unmap(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir)`
- `status_t dma_sync_for_device(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir)`
- `status_t dma_sync_for_cpu(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir)`
- `status_t dma_set_constraints(u64 device_id, const dma_constraints_t *constraints)`
- `status_t dma_get_constraints(u64 device_id, dma_constraints_t *out_constraints)`

## Notes

- Current baseline uses identity mapping and explicit status contracts.
- This is the stable substrate API; architecture-specific IOMMU/coherency behavior can be upgraded behind it.
