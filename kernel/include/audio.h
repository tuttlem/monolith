#ifndef KERNEL_AUDIO_H
#define KERNEL_AUDIO_H

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
  u64 has_output;
  u64 has_input;
  u64 min_sample_rate_hz;
  u64 max_sample_rate_hz;
} audio_device_info_t;

status_t audio_enumerate(const boot_info_t *boot_info);
u64 audio_device_count(void);
status_t audio_device_info_at(u64 index, audio_device_info_t *out_info);
void audio_dump_diagnostics(void);

#endif
