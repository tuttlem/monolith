# Unified Device Discovery (Spec 110)

API: `kernel/include/hw_desc.h`  
Implementation:
- `kernel/discovery/hw_discovery.c`
- `kernel/discovery/acpi_parser.c`
- `kernel/discovery/dtb_parser.c`

## Goal

Normalize ACPI/DT/fallback data into one early hardware descriptor consumed by generic subsystems.

## Unified Model

`hw_desc_t` currently exposes:
- `cpu_count`
- `timer_count`
- `irq_controller_count`
- `mmio_region_count`
- `uart_count`
- fixed-size descriptor arrays (`cpus`, `timers`, `irq_controllers`, `mmio_regions`, `uarts`)
- `source_mask` indicating fallback/ACPI/DT origins

## Lifecycle

1. `hw_discovery_init(const boot_info_t *boot_info)` in `kmain`
2. consumers query:
   - `const hw_desc_t *hw_desc_get(void)`
   - `BOOT_U64 hw_desc_cpu_count_hint(void)`

## Parser Split

Source-specific parsers:
- ACPI parser:
  - consumes RSDP/XSDT/RSDT
  - currently parses MADT CPU/IOAPIC data and basic timer table presence
- DT parser:
  - consumes DTB structure block
  - extracts CPU node count/ids and basic controller/timer/uart hints

Graceful fallback:
- if parsers are absent/fail, fallback still publishes:
  - boot CPU
  - architecture-default controller type
  - at least one timer slot
  - MMIO regions from normalized boot memory map
  - serial port when available

## Current Consumers

- SMP enumeration seed in `kernel/smp.c` (`hw_desc_cpu_count_hint`)
- interrupt setup gate in `kernel/interrupts.c` (descriptor presence)
- timer setup gate in `kernel/timebase.c` (descriptor presence)

This keeps architecture-specific probe logic out of generic init flow.
