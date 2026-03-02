#ifndef KERNEL_DEVICE_REPORT_H
#define KERNEL_DEVICE_REPORT_H

#include "device_bus.h"

typedef struct {
  BOOT_U64 id;
  BOOT_U64 parent_id;
  BOOT_U64 bus_id;
  const char *name;
  device_class_t class_id;
  BOOT_U64 vendor_id;
  BOOT_U64 device_id;
  BOOT_U64 class_code;
  BOOT_U64 subclass_code;
  BOOT_U64 prog_if;
  BOOT_U64 resource_count;
} device_report_entry_t;

BOOT_U64 device_report_count(void);
status_t device_report_get(BOOT_U64 index, device_report_entry_t *out_entry);
void device_report_dump_all(void);
void device_report_dump_class(device_class_t class_id);

#endif
