# 080 MMU Mapping API

## Goal

Add architecture-neutral mapping primitives before introducing advanced VM policy.

## API Surface (Target)

```c
typedef u64 mm_phys_addr_t;
typedef u64 mm_virt_addr_t;

typedef enum {
  MMU_PROT_READ  = 1 << 0,
  MMU_PROT_WRITE = 1 << 1,
  MMU_PROT_EXEC  = 1 << 2,
  MMU_PROT_USER  = 1 << 3,
  MMU_PROT_DEVICE = 1 << 4,
  MMU_PROT_GLOBAL = 1 << 5,
} mmu_prot_t;

status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, u64 size, u64 prot_flags);
status_t mm_unmap(mm_virt_addr_t va, u64 size);
status_t mm_protect(mm_virt_addr_t va, u64 size, u64 prot_flags);
status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags);
status_t mm_sync_tlb(mm_virt_addr_t va, u64 size);
```

## Design Requirements

- page-size aware alignment checks
- explicit partial-failure behavior for multi-page ops
- shared virtual memory attribute model mapped per architecture

## Architecture Backend Responsibilities

### x86_64
- page table walker/editor for 4K mappings (large-page optional)
- NX/write/user/global mapping translation

### arm64
- stage-1 PTE attribute mapping from generic flags
- memory attribute index handling (normal/device)

### riscv64
- Sv39 PTE encoding/decoding for R/W/X/U bits
- platform-safe TLB synchronization hooks

## Integration Requirements

- allocator not embedded in mapping API (mapping API consumes pages from existing allocator or pre-reserved pools)
- API must be callable before scheduler/user-mode exists

## Acceptance Criteria

- map/unmap/protect/translate pass architecture-agnostic tests
- existing early identity map remains functional after introducing API
