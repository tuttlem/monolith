#ifndef KERNEL_HW_DESC_H
#define KERNEL_HW_DESC_H

#include "kernel.h"

#define HW_DESC_API_VERSION_MAJOR 1U
#define HW_DESC_API_VERSION_MINOR 0U

#define HW_DESC_MAX_CPUS 64U
#define HW_DESC_MAX_TIMERS 8U
#define HW_DESC_MAX_IRQ_CONTROLLERS 8U
#define HW_DESC_MAX_MMIO_REGIONS 64U
#define HW_DESC_MAX_UARTS 8U

#define HW_DESC_SOURCE_FALLBACK (1ULL << 0)
#define HW_DESC_SOURCE_ACPI (1ULL << 1)
#define HW_DESC_SOURCE_DTB (1ULL << 2)

#define HW_IRQ_CONTROLLER_UNKNOWN 0ULL
#define HW_IRQ_CONTROLLER_X86_PIC 1ULL
#define HW_IRQ_CONTROLLER_X86_IOAPIC 2ULL
#define HW_IRQ_CONTROLLER_ARM_GIC 3ULL
#define HW_IRQ_CONTROLLER_RISCV_PLIC 4ULL

typedef struct {
  u64 cpu_id;
  u64 flags;
} hw_cpu_desc_t;

typedef struct {
  u64 type;
  u64 mmio_base;
  u64 mmio_size;
  u64 irq_base;
  u64 irq_count;
} hw_irq_controller_desc_t;

typedef struct {
  u64 mmio_base;
  u64 mmio_size;
  u64 irq;
  u64 freq_hz;
  u64 flags;
} hw_timer_desc_t;

typedef struct {
  u64 base;
  u64 size;
} hw_mmio_region_desc_t;

typedef struct {
  u64 base;
  u64 irq;
  u64 flags;
} hw_uart_desc_t;

typedef struct {
  u64 source_mask;
  u64 cpu_count;
  u64 timer_count;
  u64 irq_controller_count;
  u64 mmio_region_count;
  u64 uart_count;
  hw_cpu_desc_t cpus[HW_DESC_MAX_CPUS];
  hw_timer_desc_t timers[HW_DESC_MAX_TIMERS];
  hw_irq_controller_desc_t irq_controllers[HW_DESC_MAX_IRQ_CONTROLLERS];
  hw_mmio_region_desc_t mmio_regions[HW_DESC_MAX_MMIO_REGIONS];
  hw_uart_desc_t uarts[HW_DESC_MAX_UARTS];
} hw_desc_t;

status_t hw_discovery_init(const boot_info_t *boot_info);
const hw_desc_t *hw_desc_get(void);
u64 hw_desc_cpu_count_hint(void);

#endif
