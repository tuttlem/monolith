#include "device_domains.h"
#include "capability_profile.h"
#include "driver_queue.h"
#include "print.h"

static BOOT_U64 g_block_count;
static BOOT_U64 g_input_count;
static BOOT_U64 g_display_count;
static driver_ring_t g_block_probe_ring;

static int is_domain_source(const device_t *d) {
  if (d->class_id == DEVICE_CLASS_BLOCK || d->class_id == DEVICE_CLASS_INPUT || d->class_id == DEVICE_CLASS_DISPLAY) {
    return 0;
  }
  return 1;
}

static status_t add_domain_device(const device_t *src, device_class_t class_id, const char *name) {
  device_t d;
  BOOT_U64 r;

  d.id = DEVICE_BUS_ID_NONE;
  d.parent_id = src->id;
  d.bus_id = src->bus_id;
  d.name = name;
  d.class_id = class_id;
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

  return device_bus_register_device(&d, (BOOT_U64 *)0);
}

status_t device_domains_enumerate(const boot_info_t *boot_info) {
  BOOT_U64 i;
  status_t st = STATUS_DEFERRED;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  g_block_count = 0;
  g_input_count = 0;
  g_display_count = 0;
  driver_ring_init(&g_block_probe_ring, 65ULL);

  for (i = 0; i < device_bus_count(); ++i) {
    const device_t *src = device_bus_device_at(i);

    if (src == (const device_t *)0 || !is_domain_source(src)) {
      continue;
    }

    if (src->class_id == DEVICE_CLASS_PCI_DEVICE && src->class_code == 0x01ULL) {
      BOOT_U64 block_slot;
      if (!capability_domain_enabled(DEVICE_CLASS_BLOCK)) {
        continue;
      }
      if (!driver_ring_push(&g_block_probe_ring, &block_slot)) {
        continue;
      }
      if (add_domain_device(src, DEVICE_CLASS_BLOCK, "block-device") == STATUS_OK) {
        g_block_count += 1ULL;
        st = STATUS_OK;
      }
      (void)driver_ring_pop(&g_block_probe_ring, (BOOT_U64 *)0);
      continue;
    }

    if ((src->class_id == DEVICE_CLASS_PCI_DEVICE && src->class_code == 0x03ULL) ||
        src->class_id == DEVICE_CLASS_FRAMEBUFFER) {
      if (!capability_domain_enabled(DEVICE_CLASS_DISPLAY)) {
        continue;
      }
      if (add_domain_device(src, DEVICE_CLASS_DISPLAY, "display-device") == STATUS_OK) {
        g_display_count += 1ULL;
        st = STATUS_OK;
      }
      continue;
    }

    if ((src->class_id == DEVICE_CLASS_PCI_DEVICE && src->class_code == 0x09ULL) ||
        (src->class_id == DEVICE_CLASS_USB_DEVICE && src->class_code == 0x03ULL)) {
      if (!capability_domain_enabled(DEVICE_CLASS_INPUT)) {
        continue;
      }
      if (add_domain_device(src, DEVICE_CLASS_INPUT, "input-device") == STATUS_OK) {
        g_input_count += 1ULL;
        st = STATUS_OK;
      }
      continue;
    }
  }

  kprintf("domains: block=%llu input=%llu display=%llu\n", g_block_count, g_input_count, g_display_count);
  return st;
}

BOOT_U64 block_device_count(void) { return g_block_count; }
BOOT_U64 input_device_count(void) { return g_input_count; }
BOOT_U64 display_device_count(void) { return g_display_count; }
