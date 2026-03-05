#ifndef KERNEL_DEVICE_REPORT_H
#define KERNEL_DEVICE_REPORT_H

#include "device_bus.h"

typedef struct {
  u64 id;
  u64 parent_id;
  u64 bus_id;
  const char *name;
  device_class_t class_id;
  u64 vendor_id;
  u64 device_id;
  u64 class_code;
  u64 subclass_code;
  u64 prog_if;
  u64 resource_count;
} device_report_entry_t;

u64 device_report_count(void);
status_t device_report_get(u64 index, device_report_entry_t *out_entry);
void device_report_dump_all(void);
void device_report_dump_class(device_class_t class_id);

#endif
