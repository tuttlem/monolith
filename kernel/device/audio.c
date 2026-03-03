#include "audio.h"
#include "capability_profile.h"
#include "print.h"

#define AUDIO_MAX_DEVICES 32U

static BOOT_U64 g_audio_count;
static audio_device_info_t g_audio_devices[AUDIO_MAX_DEVICES];

status_t audio_enumerate(const boot_info_t *boot_info) {
  BOOT_U64 i;
  status_t st = STATUS_DEFERRED;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (!capability_domain_enabled(DEVICE_CLASS_AUDIO)) {
    g_audio_count = 0;
    return STATUS_DEFERRED;
  }

  g_audio_count = 0;

  for (i = 0; i < device_bus_count(); ++i) {
    const device_t *src = device_bus_device_at(i);
    device_t d;
    BOOT_U64 r;

    if (src == (const device_t *)0 || src->class_id != DEVICE_CLASS_PCI_DEVICE || src->class_code != 0x04ULL) {
      continue;
    }

    d.id = DEVICE_BUS_ID_NONE;
    d.parent_id = src->id;
    d.bus_id = src->bus_id;
    d.name = "audio-device";
    d.class_id = DEVICE_CLASS_AUDIO;
    d.vendor_id = src->vendor_id;
    d.device_id = src->device_id;
    d.revision = src->revision;
    d.class_code = src->class_code;
    d.subclass_code = src->subclass_code;
    d.prog_if = src->prog_if;
    d.resource_count = src->resource_count;
    d.driver_data = (void *)0;
    if (d.resource_count > DEVICE_BUS_MAX_RESOURCES) {
      d.resource_count = DEVICE_BUS_MAX_RESOURCES;
    }
    for (r = 0; r < DEVICE_BUS_MAX_RESOURCES; ++r) {
      d.resources[r].kind = src->resources[r].kind;
      d.resources[r].base = src->resources[r].base;
      d.resources[r].size = src->resources[r].size;
      d.resources[r].flags = src->resources[r].flags;
    }

    {
      BOOT_U64 new_id = DEVICE_BUS_ID_NONE;
      if (device_bus_register_device(&d, &new_id) == STATUS_OK) {
        if (g_audio_count < AUDIO_MAX_DEVICES) {
          g_audio_devices[g_audio_count].device_id = new_id;
          g_audio_devices[g_audio_count].parent_id = src->id;
          g_audio_devices[g_audio_count].vendor_id = src->vendor_id;
          g_audio_devices[g_audio_count].pci_device_id = src->device_id;
          g_audio_devices[g_audio_count].class_code = src->class_code;
          g_audio_devices[g_audio_count].subclass_code = src->subclass_code;
          g_audio_devices[g_audio_count].prog_if = src->prog_if;
          g_audio_devices[g_audio_count].resource_count = src->resource_count;
          g_audio_devices[g_audio_count].has_output = 1;
          g_audio_devices[g_audio_count].has_input = 1;
          g_audio_devices[g_audio_count].min_sample_rate_hz = 8000;
          g_audio_devices[g_audio_count].max_sample_rate_hz = 48000;
        }
        g_audio_count += 1ULL;
        st = STATUS_OK;
      }
    }
  }

  kprintf("audio: devices=%llu\n", g_audio_count);
  return st;
}

BOOT_U64 audio_device_count(void) { return g_audio_count; }

status_t audio_device_info_at(BOOT_U64 index, audio_device_info_t *out_info) {
  if (out_info == (audio_device_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (index >= g_audio_count || index >= AUDIO_MAX_DEVICES) {
    return STATUS_NOT_FOUND;
  }
  out_info->device_id = g_audio_devices[index].device_id;
  out_info->parent_id = g_audio_devices[index].parent_id;
  out_info->vendor_id = g_audio_devices[index].vendor_id;
  out_info->pci_device_id = g_audio_devices[index].pci_device_id;
  out_info->class_code = g_audio_devices[index].class_code;
  out_info->subclass_code = g_audio_devices[index].subclass_code;
  out_info->prog_if = g_audio_devices[index].prog_if;
  out_info->resource_count = g_audio_devices[index].resource_count;
  out_info->has_output = g_audio_devices[index].has_output;
  out_info->has_input = g_audio_devices[index].has_input;
  out_info->min_sample_rate_hz = g_audio_devices[index].min_sample_rate_hz;
  out_info->max_sample_rate_hz = g_audio_devices[index].max_sample_rate_hz;
  return STATUS_OK;
}

void audio_dump_diagnostics(void) {
  BOOT_U64 i;
  BOOT_U64 capped = g_audio_count;
  if (capped > AUDIO_MAX_DEVICES) {
    capped = AUDIO_MAX_DEVICES;
  }
  for (i = 0; i < capped; ++i) {
    const audio_device_info_t *a = &g_audio_devices[i];
    kprintf("audio[%llu]: ven=0x%llx dev=0x%llx out=%llu in=%llu rate=%llu-%llu\n", i, a->vendor_id,
            a->pci_device_id, a->has_output, a->has_input, a->min_sample_rate_hz, a->max_sample_rate_hz);
  }
}
