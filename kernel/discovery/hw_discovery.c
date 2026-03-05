#include "discovery_internal.h"

static hw_desc_t g_hw_desc;
static u64 g_hw_desc_initialized = 0;

static void clear_desc(hw_desc_t *desc) {
  u64 i;

  desc->source_mask = 0;
  desc->cpu_count = 0;
  desc->timer_count = 0;
  desc->irq_controller_count = 0;
  desc->mmio_region_count = 0;
  desc->uart_count = 0;

  for (i = 0; i < HW_DESC_MAX_CPUS; ++i) {
    desc->cpus[i].cpu_id = 0;
    desc->cpus[i].flags = 0;
  }
  for (i = 0; i < HW_DESC_MAX_TIMERS; ++i) {
    desc->timers[i].mmio_base = 0;
    desc->timers[i].mmio_size = 0;
    desc->timers[i].irq = 0;
    desc->timers[i].freq_hz = 0;
    desc->timers[i].flags = 0;
  }
  for (i = 0; i < HW_DESC_MAX_IRQ_CONTROLLERS; ++i) {
    desc->irq_controllers[i].type = HW_IRQ_CONTROLLER_UNKNOWN;
    desc->irq_controllers[i].mmio_base = 0;
    desc->irq_controllers[i].mmio_size = 0;
    desc->irq_controllers[i].irq_base = 0;
    desc->irq_controllers[i].irq_count = 0;
  }
  for (i = 0; i < HW_DESC_MAX_MMIO_REGIONS; ++i) {
    desc->mmio_regions[i].base = 0;
    desc->mmio_regions[i].size = 0;
  }
  for (i = 0; i < HW_DESC_MAX_UARTS; ++i) {
    desc->uarts[i].base = 0;
    desc->uarts[i].irq = 0;
    desc->uarts[i].flags = 0;
  }
}

static void add_fallback_cpu(hw_desc_t *desc, u64 cpu_id) {
  if (desc->cpu_count >= HW_DESC_MAX_CPUS) {
    return;
  }
  desc->cpus[desc->cpu_count].cpu_id = cpu_id;
  desc->cpus[desc->cpu_count].flags = 0;
  desc->cpu_count += 1ULL;
}

static void add_fallback_irqc(hw_desc_t *desc, u64 type) {
  if (desc->irq_controller_count >= HW_DESC_MAX_IRQ_CONTROLLERS) {
    return;
  }
  desc->irq_controllers[desc->irq_controller_count].type = type;
  desc->irq_controllers[desc->irq_controller_count].mmio_base = 0;
  desc->irq_controllers[desc->irq_controller_count].mmio_size = 0;
  desc->irq_controllers[desc->irq_controller_count].irq_base = 0;
  desc->irq_controllers[desc->irq_controller_count].irq_count = 0;
  desc->irq_controller_count += 1ULL;
}

status_t hw_discovery_init(const boot_info_t *boot_info) {
  u32 i;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  clear_desc(&g_hw_desc);
  g_hw_desc.source_mask |= HW_DESC_SOURCE_FALLBACK;

  if ((boot_info->valid_mask & BOOT_INFO_HAS_MEM_REGIONS) != 0) {
    for (i = 0; i < boot_info->memory_region_count && g_hw_desc.mmio_region_count < HW_DESC_MAX_MMIO_REGIONS; ++i) {
      const boot_mem_region_t *r = &boot_info->memory_regions[i];
      if (r->kind == BOOT_MEM_REGION_MMIO && r->size != 0) {
        g_hw_desc.mmio_regions[g_hw_desc.mmio_region_count].base = r->base;
        g_hw_desc.mmio_regions[g_hw_desc.mmio_region_count].size = r->size;
        g_hw_desc.mmio_region_count += 1ULL;
      }
    }
  }

  if (boot_info->serial_port != 0 && g_hw_desc.uart_count < HW_DESC_MAX_UARTS) {
    g_hw_desc.uarts[g_hw_desc.uart_count].base = boot_info->serial_port;
    g_hw_desc.uarts[g_hw_desc.uart_count].irq = 0;
    g_hw_desc.uarts[g_hw_desc.uart_count].flags = 0;
    g_hw_desc.uart_count += 1ULL;
  }

  if (boot_info->acpi_rsdp != 0) {
    hw_discovery_parse_acpi(boot_info, &g_hw_desc);
  }
  if (boot_info->dtb_ptr != 0) {
    hw_discovery_parse_dtb(boot_info, &g_hw_desc);
  }

  if (g_hw_desc.cpu_count == 0) {
    add_fallback_cpu(&g_hw_desc, boot_info->boot_cpu_id);
  }
  if (g_hw_desc.timer_count == 0 && g_hw_desc.timer_count < HW_DESC_MAX_TIMERS) {
    g_hw_desc.timers[0].mmio_base = 0;
    g_hw_desc.timers[0].mmio_size = 0;
    g_hw_desc.timers[0].irq = 0;
    g_hw_desc.timers[0].freq_hz = 0;
    g_hw_desc.timers[0].flags = 0;
    g_hw_desc.timer_count = 1ULL;
  }
  if (g_hw_desc.irq_controller_count == 0) {
    if (boot_info->arch_id == BOOT_INFO_ARCH_X86_64) {
      add_fallback_irqc(&g_hw_desc, HW_IRQ_CONTROLLER_X86_PIC);
    } else if (boot_info->arch_id == BOOT_INFO_ARCH_ARM64) {
      add_fallback_irqc(&g_hw_desc, HW_IRQ_CONTROLLER_ARM_GIC);
    } else if (boot_info->arch_id == BOOT_INFO_ARCH_RISCV64) {
      add_fallback_irqc(&g_hw_desc, HW_IRQ_CONTROLLER_RISCV_PLIC);
    } else {
      add_fallback_irqc(&g_hw_desc, HW_IRQ_CONTROLLER_UNKNOWN);
    }
  }

  g_hw_desc_initialized = 1ULL;
  return STATUS_OK;
}

const hw_desc_t *hw_desc_get(void) {
  if (g_hw_desc_initialized == 0) {
    return (const hw_desc_t *)0;
  }
  return &g_hw_desc;
}

u64 hw_desc_cpu_count_hint(void) {
  if (g_hw_desc_initialized == 0 || g_hw_desc.cpu_count == 0) {
    return 0ULL;
  }
  return g_hw_desc.cpu_count;
}
