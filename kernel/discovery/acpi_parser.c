#include "discovery_internal.h"

typedef struct {
  char signature[8];
  unsigned char checksum;
  char oem_id[6];
  unsigned char revision;
  unsigned int rsdt_address;
  unsigned int length;
  u64 xsdt_address;
  unsigned char extended_checksum;
  unsigned char reserved[3];
} __attribute__((packed)) rsdp_t;

typedef struct {
  char signature[4];
  unsigned int length;
  unsigned char revision;
  unsigned char checksum;
  char oem_id[6];
  char oem_table_id[8];
  unsigned int oem_revision;
  unsigned int creator_id;
  unsigned int creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
  acpi_sdt_header_t header;
  unsigned int lapic_addr;
  unsigned int flags;
} __attribute__((packed)) madt_t;

typedef struct {
  unsigned char type;
  unsigned char length;
} __attribute__((packed)) madt_entry_header_t;

typedef struct {
  madt_entry_header_t h;
  unsigned char acpi_processor_id;
  unsigned char apic_id;
  unsigned int flags;
} __attribute__((packed)) madt_lapic_t;

typedef struct {
  madt_entry_header_t h;
  unsigned short reserved;
  unsigned int ioapic_id;
  unsigned int ioapic_addr;
  unsigned int gsi_base;
} __attribute__((packed)) madt_ioapic_t;

typedef struct {
  madt_entry_header_t h;
  unsigned short reserved;
  unsigned int x2apic_id;
  unsigned int flags;
  unsigned int acpi_uid;
} __attribute__((packed)) madt_x2apic_t;

typedef struct {
  madt_entry_header_t h;
  unsigned short reserved;
  unsigned int cpu_interface_number;
  unsigned int acpi_uid;
  unsigned int flags;
  unsigned int parking_protocol_version;
  unsigned int performance_interrupt_gsiv;
  u64 parked_address;
  u64 physical_base_address;
  u64 gicv;
  u64 gich;
  unsigned int vgic_maintenance_interrupt;
  u64 gicr_base_address;
  u64 mpidr;
  unsigned char processor_power_efficiency_class;
  unsigned char reserved2[3];
} __attribute__((packed)) madt_gicc_t;

typedef struct {
  madt_entry_header_t h;
  unsigned short reserved;
  unsigned int gic_id;
  u64 physical_base_address;
  unsigned int system_vector_base;
  unsigned char gic_version;
  unsigned char reserved2[3];
} __attribute__((packed)) madt_gicd_t;

static int sig_eq4(const char *a, const char *b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static int sig_eq8(const char *a, const char *b) {
  u32 i;
  for (i = 0; i < 8U; ++i) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

static void add_cpu(hw_desc_t *desc, u64 cpu_id) {
  if (desc->cpu_count >= HW_DESC_MAX_CPUS) {
    return;
  }
  desc->cpus[desc->cpu_count].cpu_id = cpu_id;
  desc->cpus[desc->cpu_count].flags = 0;
  desc->cpu_count += 1ULL;
}

static void add_irqc(hw_desc_t *desc, u64 type, u64 base, u64 size, u64 irq_base, u64 irq_count) {
  if (desc->irq_controller_count >= HW_DESC_MAX_IRQ_CONTROLLERS) {
    return;
  }
  desc->irq_controllers[desc->irq_controller_count].type = type;
  desc->irq_controllers[desc->irq_controller_count].mmio_base = base;
  desc->irq_controllers[desc->irq_controller_count].mmio_size = size;
  desc->irq_controllers[desc->irq_controller_count].irq_base = irq_base;
  desc->irq_controllers[desc->irq_controller_count].irq_count = irq_count;
  desc->irq_controller_count += 1ULL;
}

static void parse_madt(hw_desc_t *desc, const madt_t *madt) {
  const unsigned char *p;
  const unsigned char *end;

  if (madt == (const madt_t *)0 || madt->header.length < sizeof(madt_t)) {
    return;
  }

  p = (const unsigned char *)madt + sizeof(madt_t);
  end = (const unsigned char *)madt + madt->header.length;

  while (p + sizeof(madt_entry_header_t) <= end) {
    const madt_entry_header_t *h = (const madt_entry_header_t *)p;
    if (h->length < sizeof(madt_entry_header_t) || p + h->length > end) {
      break;
    }

    if (h->type == 0 && h->length >= sizeof(madt_lapic_t)) {
      const madt_lapic_t *e = (const madt_lapic_t *)p;
      if ((e->flags & 1U) != 0U) {
        add_cpu(desc, (u64)e->apic_id);
      }
    } else if (h->type == 1 && h->length >= sizeof(madt_ioapic_t)) {
      const madt_ioapic_t *e = (const madt_ioapic_t *)p;
      add_irqc(desc, HW_IRQ_CONTROLLER_X86_IOAPIC, (u64)e->ioapic_addr, 0x20ULL, (u64)e->gsi_base, 24ULL);
    } else if (h->type == 9 && h->length >= sizeof(madt_x2apic_t)) {
      const madt_x2apic_t *e = (const madt_x2apic_t *)p;
      if ((e->flags & 1U) != 0U) {
        add_cpu(desc, (u64)e->x2apic_id);
      }
    } else if (h->type == 11 && h->length >= sizeof(madt_gicc_t)) {
      const madt_gicc_t *e = (const madt_gicc_t *)p;
      if ((e->flags & 1U) != 0U) {
        add_cpu(desc, e->mpidr);
      }
    } else if (h->type == 12 && h->length >= sizeof(madt_gicd_t)) {
      const madt_gicd_t *e = (const madt_gicd_t *)p;
      add_irqc(desc, HW_IRQ_CONTROLLER_ARM_GIC, e->physical_base_address, 0x10000ULL, 0ULL, 0ULL);
    }

    p += h->length;
  }
}

void hw_discovery_parse_acpi(const boot_info_t *boot_info, hw_desc_t *desc) {
  const rsdp_t *rsdp;
  const acpi_sdt_header_t *root;
  u64 entries;
  u64 i;

  if (boot_info == (const boot_info_t *)0 || desc == (hw_desc_t *)0 || boot_info->acpi_rsdp == 0) {
    return;
  }

  rsdp = (const rsdp_t *)(uptr)boot_info->acpi_rsdp;
  if (!sig_eq8(rsdp->signature, "RSD PTR ")) {
    return;
  }

  if (rsdp->revision >= 2U && rsdp->xsdt_address != 0) {
    root = (const acpi_sdt_header_t *)(uptr)rsdp->xsdt_address;
    if (root == (const acpi_sdt_header_t *)0 || root->length < sizeof(acpi_sdt_header_t)) {
      return;
    }
    entries = (root->length - sizeof(acpi_sdt_header_t)) / 8ULL;
    for (i = 0; i < entries; ++i) {
      const u64 *entry = (const u64 *)((const unsigned char *)root + sizeof(acpi_sdt_header_t) + i * 8ULL);
      const acpi_sdt_header_t *tbl = (const acpi_sdt_header_t *)(uptr)(*entry);
      if (tbl == (const acpi_sdt_header_t *)0 || tbl->length < sizeof(acpi_sdt_header_t)) {
        continue;
      }
      if (sig_eq4(tbl->signature, "APIC")) {
        parse_madt(desc, (const madt_t *)tbl);
      } else if (sig_eq4(tbl->signature, "GTDT")) {
        if (desc->timer_count < HW_DESC_MAX_TIMERS) {
          desc->timers[desc->timer_count].mmio_base = 0;
          desc->timers[desc->timer_count].mmio_size = 0;
          desc->timers[desc->timer_count].irq = 0;
          desc->timers[desc->timer_count].freq_hz = 0;
          desc->timers[desc->timer_count].flags = 0;
          desc->timer_count += 1ULL;
        }
      }
    }
  } else if (rsdp->rsdt_address != 0U) {
    root = (const acpi_sdt_header_t *)(uptr)((u64)rsdp->rsdt_address);
    if (root == (const acpi_sdt_header_t *)0 || root->length < sizeof(acpi_sdt_header_t)) {
      return;
    }
    entries = (root->length - sizeof(acpi_sdt_header_t)) / 4ULL;
    for (i = 0; i < entries; ++i) {
      const unsigned int *entry = (const unsigned int *)((const unsigned char *)root + sizeof(acpi_sdt_header_t) + i * 4ULL);
      const acpi_sdt_header_t *tbl = (const acpi_sdt_header_t *)(uptr)((u64)(*entry));
      if (tbl == (const acpi_sdt_header_t *)0 || tbl->length < sizeof(acpi_sdt_header_t)) {
        continue;
      }
      if (sig_eq4(tbl->signature, "APIC")) {
        parse_madt(desc, (const madt_t *)tbl);
      }
    }
  }

  desc->source_mask |= HW_DESC_SOURCE_ACPI;
}
