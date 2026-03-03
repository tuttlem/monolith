# 132 Interrupt Domains and MSI/MSI-X Abstraction

## Goal

Provide a generic interrupt allocation and mapping layer that supports line IRQ and message IRQ (MSI/MSI-X) without exposing controller-specific programming to OS-facing code.

## In Scope

- Interrupt domain abstraction:
  - global IRQ id
  - per-controller hwirq mapping
  - per-device IRQ allocation
- MSI/MSI-X abstract allocation/programming contract.
- Generic APIs for:
  - allocate IRQ vectors
  - configure trigger/polarity
  - set affinity hints
  - mask/unmask/eoi.

## Out of Scope

- Full interrupt balancing policy.
- User/kernel interrupt policy model.

## Public Interfaces

- Header: `kernel/include/irq_domain.h`
- APIs:
  - `status_t irq_domain_init(const boot_info_t *boot_info)`
  - `status_t irq_alloc_line(device_id_t dev, BOOT_U64 hwirq, irq_desc_t *out)`
  - `status_t irq_alloc_msi(device_id_t dev, BOOT_U64 nvec, irq_desc_t *out_vec)`
  - `status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg)`
  - `status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask)`

## Architecture Backends

- `x86_64`: PIC (legacy), IOAPIC/LAPIC, MSI path for PCIe.
- `arm64`: GICv2/v3 mapping, SPI/PPI support, MSI path where available.
- `riscv64`: PLIC/AIA/APLIC/IMSIC abstraction as available.

## Init Order

1. `interrupts_init`
2. controller backend init
3. `irq_domain_init`
4. timer/device IRQ allocation

## Diagnostics

- dump IRQ graph: `global_irq -> domain -> controller -> hwirq`.
- per-device allocated IRQ summary.

## Tests

- Allocate/release cycle tests.
- Timer IRQ allocation through domain API on all architectures.
- Optional MSI smoke on x86_64 (virtio/e1000 path where supported).

## Acceptance Criteria

1. Timer and at least one non-timer device IRQ path use domain APIs.
2. No driver writes controller-specific registers directly in generic code.
3. MSI allocation returns deterministic, validated descriptors.
