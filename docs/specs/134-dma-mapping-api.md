# 134 DMA Mapping API

## Goal

Provide a generic DMA mapping/unmapping layer that hides cache coherency and address translation details from drivers.

## In Scope

- DMA buffer allocation helpers (optional contiguous pool + generic pages).
- Map/unmap API for streaming DMA.
- Sync operations for non-coherent systems.
- Direction-aware semantics (`to_device`, `from_device`, `bidirectional`).
- DMA constraints (`max_addr`, `segment_len`, alignment).

## Out of Scope

- Full IOMMU policy (separate spec).

## Public Interfaces

- Header: `kernel/include/dma.h`
- APIs:
  - `status_t dma_init(const boot_info_t *boot_info)`
  - `status_t dma_map(device_id_t dev, void *cpu_ptr, u64 len, dma_dir_t dir, dma_addr_t *out)`
  - `status_t dma_unmap(device_id_t dev, dma_addr_t addr, u64 len, dma_dir_t dir)`
  - `status_t dma_sync_for_device(...)`
  - `status_t dma_sync_for_cpu(...)`

## Architecture Backends

- `x86_64`: coherent by default on common platforms; still honor API semantics.
- `arm64`: support coherent and non-coherent paths.
- `riscv64`: support platform-dependent coherency via explicit sync hooks.

## Tests

- Map/unmap lifecycle tests.
- Cache sync no-op vs non-no-op behavior checks per backend capability.
- Driver helper smoke using ring buffer descriptors.

## Acceptance Criteria

1. Block/net baseline paths use DMA API, not raw physical assumptions.
2. No generic driver cache-management assembly.
3. Clear failure statuses when constraints cannot be satisfied.
