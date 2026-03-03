# Monolith Developer Manual

This manual explains the current platform bring-up base for:
- `x86_64`
- `arm64`
- `riscv64`

Use this as the reference for boot handoff, memory initialization, MMU mapping, per-CPU runtime, allocators, interrupts, timers, and diagnostics.

## Read This First

1. [Core Interfaces](core/core-interfaces.md)
2. [Status, Assert, Panic](core/status-system.md)
3. [Config and Feature Flags](core/config-flags.md)
4. [CPU Layer (`arch_cpu`)](core/cpu-layer.md)
5. [Exception and Interrupt Framework](core/exceptions.md)
6. [Interrupt Controller Layer](core/irq-controller.md)
7. [IRQ Domains and MSI Abstraction](core/irq-domains.md)
8. [Time System](core/time-system.md)
9. [Memory Stack + MMU Mapping API](core/memory.md)
10. [DMA Mapping API](core/dma.md)
11. [IOMMU Subsystem](core/iommu.md)
12. [Clock, Power, and Reset Framework](core/clock-power-reset.md)
13. [CPU Feature and Context Facilities](core/cpu-context.md)
14. [Per-CPU Runtime](core/percpu.md)
15. [SMP Bootstrap Skeleton](core/smp-bootstrap.md)
16. [Unified Device Discovery](core/device-discovery.md)
17. [Device Model Baseline](core/device-model.md)
18. [Device Bus Core](core/device-bus.md)
19. [Hardware Resource Manager](core/hardware-resources.md)
20. [PCI Enumeration](core/pci-enumeration.md)
21. [USB Enumeration Skeleton](core/usb-enumeration.md)
22. [Device Domain Baseline](core/device-domains.md)
23. [Device Reporting and Introspection](core/device-reporting.md)
24. [Capability Profiles and Feature Gating](core/capability-profiles.md)
25. [Network Baseline (Optional Domain)](core/network-baseline.md)
26. [Audio Baseline (Optional Domain)](core/audio-baseline.md)
27. [Standard Capability Domains](core/standard-capability-domains.md)
28. [Syscall Transport ABI](core/syscall-transport.md)
29. [Core Boot Sequence](core/boot-sequence.md)
30. [Boot ABI (`boot_info_t`)](core/boot-info-abi.md)
31. [Core API Reference](core/api-reference.md)
32. [Core Data Structures and Enums](core/data-structures.md)
33. [Architecture Divergence Guide](core/architecture-divergence.md)
34. [Implementation Coverage](core/implementation-coverage.md)
35. [API Cheatsheet](core/api-cheatsheet.md)

## Architecture-Specific Manuals

- [x86_64 Manual](x86_64/README.md)
- [arm64 Manual](arm64/README.md)
- [riscv64 Manual](riscv64/README.md)

## Practical Usage Model

When starting a new OS on top of this base, your normal first integrations are:
1. Keep boot protocol and `boot_info_t` stable.
2. Bring up CPU-local runtime (`arch_cpu_early_init`, `percpu_init_boot_cpu`).
3. Bring up memory (`arch_mm_early_init`, `page_alloc_init`, `kmalloc_init`).
4. Bring up interrupts and timer (`interrupts_init`, `timer_init`).
5. Add your platform-independent subsystems on top of `kmalloc`, `status_t`, per-CPU accessors, and interrupt/timer callbacks.

The current code path that orchestrates this is `kmain` in `kernel/kmain.c`.
