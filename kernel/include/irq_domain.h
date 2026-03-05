#ifndef KERNEL_IRQ_DOMAIN_H
#define KERNEL_IRQ_DOMAIN_H

#include "boot_info.h"
#include "status.h"

#define IRQ_DOMAIN_MAX_ALLOCS 256U

typedef struct {
  u64 global_irq;
  u64 hwirq;
  u64 vector;
  u64 flags;
  u64 owner_device_id;
} irq_desc_t;

typedef struct {
  u64 trigger_flags;
  u64 polarity_flags;
} irq_cfg_t;

typedef u64 cpu_mask_t;

status_t irq_domain_init(const boot_info_t *boot_info);
status_t irq_alloc_line(u64 device_id, u64 hwirq, irq_desc_t *out);
status_t irq_alloc_msi(u64 device_id, u64 nvec, irq_desc_t *out_vec);
status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg);
status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask);
u64 irq_domain_alloc_count(void);

#endif
