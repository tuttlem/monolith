#ifndef KERNEL_AUDIO_H
#define KERNEL_AUDIO_H

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
  BOOT_U64 has_output;
  BOOT_U64 has_input;
  BOOT_U64 min_sample_rate_hz;
  BOOT_U64 max_sample_rate_hz;
} audio_device_info_t;

status_t audio_enumerate(const boot_info_t *boot_info);
BOOT_U64 audio_device_count(void);
status_t audio_device_info_at(BOOT_U64 index, audio_device_info_t *out_info);
void audio_dump_diagnostics(void);

#endif
