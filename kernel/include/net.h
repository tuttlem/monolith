#ifndef KERNEL_NET_H
#define KERNEL_NET_H

#include "device_bus.h"

typedef struct {
  BOOT_U64 device_id;
  BOOT_U64 parent_id;
  BOOT_U64 vendor_id;
  BOOT_U64 pci_device_id;
  BOOT_U64 class_code;
  BOOT_U64 subclass_code;
  BOOT_U64 prog_if;
  BOOT_U64 resource_count;
  BOOT_U64 mac_hi;
  BOOT_U64 mac_lo;
  BOOT_U64 link_up;
  BOOT_U64 tx_queue_count;
  BOOT_U64 rx_queue_count;
} net_device_info_t;

status_t net_enumerate(const boot_info_t *boot_info);
BOOT_U64 net_device_count(void);
status_t net_device_info_at(BOOT_U64 index, net_device_info_t *out_info);
void net_dump_diagnostics(void);

#endif
