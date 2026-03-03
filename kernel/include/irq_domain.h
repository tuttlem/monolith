#ifndef KERNEL_IRQ_DOMAIN_H
#define KERNEL_IRQ_DOMAIN_H

#include "boot_info.h"
#include "status.h"

#define IRQ_DOMAIN_MAX_ALLOCS 256U

typedef struct {
  BOOT_U64 global_irq;
  BOOT_U64 hwirq;
  BOOT_U64 vector;
  BOOT_U64 flags;
  BOOT_U64 owner_device_id;
} irq_desc_t;

typedef struct {
  BOOT_U64 trigger_flags;
  BOOT_U64 polarity_flags;
} irq_cfg_t;

typedef BOOT_U64 cpu_mask_t;

status_t irq_domain_init(const boot_info_t *boot_info);
status_t irq_alloc_line(BOOT_U64 device_id, BOOT_U64 hwirq, irq_desc_t *out);
status_t irq_alloc_msi(BOOT_U64 device_id, BOOT_U64 nvec, irq_desc_t *out_vec);
status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg);
status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask);
BOOT_U64 irq_domain_alloc_count(void);

#endif
