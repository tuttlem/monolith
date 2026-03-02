# Base Platform Specifications

This directory defines the target hardware substrate for Monolith as a reusable kernel base, not a full OS.

Architectures in scope:
- `x86_64`
- `arm64`
- `riscv64`

## Design Objectives

1. Freeze stable kernel-facing interfaces.
2. Keep architecture-specific code behind strict backend boundaries.
3. Reach feature parity at the substrate layer before OS policy work.
4. Ensure each layer has explicit init order and acceptance tests.

## Workflow Assets

- [Foundation Phase Gates](FOUNDATION-GATES.md)
- [Spec Implementation Template](SPEC-IMPLEMENTATION-TEMPLATE.md)

## Ordered Execution Plan

1. [000 Foundation and Rules](000-foundation.md)
2. [010 Stable Core Interfaces and Contracts](010-core-interfaces.md)
3. [020 Status, Assertions, and Panic Policy](020-status-panic-policy.md)
4. [030 Configuration and Feature Flag Model](030-config-feature-flags.md)
5. [040 Architecture CPU Layer (`arch_cpu`)](040-arch-cpu.md)
6. [050 Exception and Interrupt Framework](050-exception-interrupt-framework.md)
7. [060 Interrupt Controller Drivers](060-interrupt-controllers.md)
8. [070 Time System: Clocksource + Clockevent](070-time-system.md)
9. [080 MMU Mapping API](080-mmu-mapping-api.md)
10. [090 CPU-Local Data and Per-CPU Runtime](090-per-cpu.md)
11. [100 SMP Bootstrap Skeleton](100-smp-bootstrap.md)
12. [110 Unified Device Discovery (ACPI/DT)](110-device-discovery.md)
13. [120 Device Model Baseline](120-device-model.md)
14. [121 Bus Core and Device Graph](121-bus-core.md)
15. [122 PCI/PCIe Enumeration](122-pci-enumeration.md)
16. [123 USB Enumeration Skeleton](123-usb-enumeration-skeleton.md)
17. [124 Block, Input, and Display Baseline Drivers](124-block-input-display-baseline.md)
18. [125 Device Reporting and Introspection](125-device-reporting-and-introspection.md)
19. [126 Capability Profiles and Feature Gating](126-capability-profiles-and-gating.md)
20. [127 Network Baseline (Optional Domain)](127-network-baseline.md)
21. [128 Audio Baseline (Optional Domain)](128-audio-baseline.md)
22. [129 Standard Capability Domains for OS Writers](129-standard-capability-domains.md)
23. [130 Syscall ABI Skeleton](130-syscall-abi.md)
24. [140 Scheduler Scaffolding](140-scheduler-scaffolding.md)
25. [150 VFS Contract (Minimal)](150-vfs-contract.md)
26. [160 Build/Test and Bring-up Discipline](160-build-test-discipline.md)
27. [Architecture Roadmap and Parity Matrix](170-roadmap-by-arch.md)
28. [OS Layer Next Steps](200-os-layer-next-steps.md)

## Current State vs Target

Current code already has:
- `boot_info_t` handoff
- status model (`status_t`)
- early memory init + page allocator + kmalloc
- interrupt dispatch and timer skeleton

These specs define what must be added/refined to make this a durable starting platform for multiple OS designs.
