#include "net.h"
#include "capability_profile.h"
#include "print.h"

#define NET_MAX_DEVICES 32U

static BOOT_U64 g_net_count;
static net_device_info_t g_net_devices[NET_MAX_DEVICES];

status_t net_enumerate(const boot_info_t *boot_info) {
  BOOT_U64 i;
  status_t st = STATUS_DEFERRED;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (!capability_domain_enabled(DEVICE_CLASS_NET)) {
    g_net_count = 0;
    return STATUS_DEFERRED;
  }

  g_net_count = 0;

  for (i = 0; i < device_bus_count(); ++i) {
    const device_t *src = device_bus_device_at(i);
    device_t d;
    BOOT_U64 r;

    if (src == (const device_t *)0 || src->class_id != DEVICE_CLASS_PCI_DEVICE || src->class_code != 0x02ULL) {
      continue;
    }

    d.id = DEVICE_BUS_ID_NONE;
    d.parent_id = src->id;
    d.bus_id = src->bus_id;
    d.name = "net-device";
    d.class_id = DEVICE_CLASS_NET;
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
      if (g_net_count < NET_MAX_DEVICES) {
        g_net_devices[g_net_count].device_id = new_id;
        g_net_devices[g_net_count].parent_id = src->id;
        g_net_devices[g_net_count].vendor_id = src->vendor_id;
        g_net_devices[g_net_count].pci_device_id = src->device_id;
        g_net_devices[g_net_count].class_code = src->class_code;
        g_net_devices[g_net_count].subclass_code = src->subclass_code;
        g_net_devices[g_net_count].prog_if = src->prog_if;
        g_net_devices[g_net_count].resource_count = src->resource_count;
        g_net_devices[g_net_count].mac_hi = 0;
        g_net_devices[g_net_count].mac_lo = 0;
        g_net_devices[g_net_count].link_up = 0;
        g_net_devices[g_net_count].tx_queue_count = 1;
        g_net_devices[g_net_count].rx_queue_count = 1;
      }
      g_net_count += 1ULL;
      st = STATUS_OK;
    }
    }
  }

  kprintf("net: devices=%llu link_up=0\n", g_net_count);
  return st;
}

BOOT_U64 net_device_count(void) { return g_net_count; }

status_t net_device_info_at(BOOT_U64 index, net_device_info_t *out_info) {
  if (out_info == (net_device_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (index >= g_net_count || index >= NET_MAX_DEVICES) {
    return STATUS_NOT_FOUND;
  }
  out_info->device_id = g_net_devices[index].device_id;
  out_info->parent_id = g_net_devices[index].parent_id;
  out_info->vendor_id = g_net_devices[index].vendor_id;
  out_info->pci_device_id = g_net_devices[index].pci_device_id;
  out_info->class_code = g_net_devices[index].class_code;
  out_info->subclass_code = g_net_devices[index].subclass_code;
  out_info->prog_if = g_net_devices[index].prog_if;
  out_info->resource_count = g_net_devices[index].resource_count;
  out_info->mac_hi = g_net_devices[index].mac_hi;
  out_info->mac_lo = g_net_devices[index].mac_lo;
  out_info->link_up = g_net_devices[index].link_up;
  out_info->tx_queue_count = g_net_devices[index].tx_queue_count;
  out_info->rx_queue_count = g_net_devices[index].rx_queue_count;
  return STATUS_OK;
}

void net_dump_diagnostics(void) {
  BOOT_U64 i;
  BOOT_U64 capped = g_net_count;
  if (capped > NET_MAX_DEVICES) {
    capped = NET_MAX_DEVICES;
  }
  for (i = 0; i < capped; ++i) {
    const net_device_info_t *n = &g_net_devices[i];
    kprintf("net[%llu]: ven=0x%llx dev=0x%llx link=%llu txq=%llu rxq=%llu res=%llu\n", i, n->vendor_id,
            n->pci_device_id, n->link_up, n->tx_queue_count, n->rx_queue_count, n->resource_count);
  }
}
