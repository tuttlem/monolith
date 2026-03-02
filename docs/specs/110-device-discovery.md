# 110 Unified Device Discovery (ACPI/DT)

## Goal

Normalize firmware/hardware description data into one internal model consumed by irq/timer/device setup.

## Inputs

- ACPI tables (x86_64 and possibly arm64 UEFI systems)
- Device Tree Blob (riscv64 and some arm64 environments)

## Unified Output Model

Create discovery descriptors for:
- CPUs
- interrupt controllers
- timer devices
- MMIO ranges
- UART/console

Suggested core type:

```c
typedef struct {
  BOOT_U64 cpu_count;
  BOOT_U64 timer_count;
  BOOT_U64 irq_controller_count;
  BOOT_U64 mmio_region_count;
  // fixed-size arrays for early bring-up; dynamic extension later
} hw_desc_t;
```

## Design Rules

- parsers are source-specific modules (ACPI parser, DT parser)
- consumers use only unified descriptors
- parser failure must degrade gracefully when fallback data exists

## Initial Consumer Targets

1. interrupt-controller initialization
2. timer source selection
3. SMP CPU enumeration
4. serial/framebuffer registration

## Acceptance Criteria

- all three architectures produce at least minimal valid `hw_desc_t`
- controller/timer setup paths consume unified descriptors rather than hardcoded addresses where possible
