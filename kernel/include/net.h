#ifndef KERNEL_NET_H
#define KERNEL_NET_H

#include "device_bus.h"

typedef struct {
  u64 device_id;
  u64 parent_id;
  u64 vendor_id;
  u64 pci_device_id;
  u64 class_code;
  u64 subclass_code;
  u64 prog_if;
  u64 resource_count;
  u64 mac_hi;
  u64 mac_lo;
  u64 link_up;
  u64 tx_queue_count;
  u64 rx_queue_count;
} net_device_info_t;

status_t net_enumerate(const boot_info_t *boot_info);
u64 net_device_count(void);
status_t net_device_info_at(u64 index, net_device_info_t *out_info);
void net_dump_diagnostics(void);

#endif
