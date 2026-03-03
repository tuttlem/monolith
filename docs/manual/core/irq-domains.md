# IRQ Domains and MSI Abstraction

This layer adds generic IRQ allocation interfaces on top of the active interrupt-controller backend.

## Purpose

- Allocate line IRQ descriptors through a stable interface.
- Reserve an API shape for MSI/MSI-X without exposing controller internals.
- Keep device-level IRQ management architecture-neutral.

## Public API

Header: `kernel/include/irq_domain.h`

- `status_t irq_domain_init(const boot_info_t *boot_info)`
- `status_t irq_alloc_line(BOOT_U64 device_id, BOOT_U64 hwirq, irq_desc_t *out)`
- `status_t irq_alloc_msi(BOOT_U64 device_id, BOOT_U64 nvec, irq_desc_t *out_vec)`
- `status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg)`
- `status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask)`
- `BOOT_U64 irq_domain_alloc_count(void)`

## Notes

- Current implementation provides line-IRQ allocation via `irq_controller_map`.
- MSI allocation now returns descriptor reservations immediately (`flags` includes MSI marker), so higher layers can bind vectors/capabilities without a controller-specific API leak.
