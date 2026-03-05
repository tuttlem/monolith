#include "pci.h"
#include "print.h"

static u64 g_pci_device_count;

#if defined(__x86_64__)
static u64 g_pci_bus_id = DEVICE_BUS_ID_NONE;
static void outl(unsigned short port, unsigned int value) {
  __asm__ volatile("outl %0, %1" : : "a"(value), "dN"(port));
}

static unsigned int inl(unsigned short port) {
  unsigned int value;
  __asm__ volatile("inl %1, %0" : "=a"(value) : "dN"(port));
  return value;
}

static unsigned int pci_cfg_read32(u64 bus, u64 slot, u64 func, u64 off) {
  unsigned int address = 0x80000000U | ((unsigned int)(bus & 0xFFULL) << 16) | ((unsigned int)(slot & 0x1FULL) << 11) |
                         ((unsigned int)(func & 0x7ULL) << 8) | (unsigned int)(off & 0xFCULL);
  outl(0xCF8U, address);
  return inl(0xCFCU);
}

static u64 header_type(u64 bus, u64 slot, u64 func) {
  unsigned int v = pci_cfg_read32(bus, slot, func, 0x0CU);
  return (u64)((v >> 16) & 0xFFU);
}

static u64 read_vendor(u64 bus, u64 slot, u64 func) {
  return (u64)(pci_cfg_read32(bus, slot, func, 0x00U) & 0xFFFFU);
}

static status_t ensure_pci_bus(void) {
  bus_t bus;
  status_t st;

  if (g_pci_bus_id != DEVICE_BUS_ID_NONE) {
    return STATUS_OK;
  }

  bus.id = DEVICE_BUS_ID_NONE;
  bus.parent_bus_id = 0;
  bus.type = BUS_TYPE_PCI;
  bus.name = "pci";
  st = device_bus_register_bus(&bus, &g_pci_bus_id);
  return st;
}

static void fill_bar_resources(device_t *dev, u64 bus, u64 slot, u64 func, u64 hdr_type) {
  u64 max_bars = ((hdr_type & 0x7fULL) == 0ULL) ? 6ULL : 2ULL;
  u64 bar;

  for (bar = 0; bar < max_bars && dev->resource_count < DEVICE_BUS_MAX_RESOURCES; ++bar) {
    unsigned int raw = pci_cfg_read32(bus, slot, func, 0x10U + bar * 4ULL);
    if (raw == 0U || raw == 0xFFFFFFFFU) {
      continue;
    }
    if ((raw & 1U) != 0U) {
      dev->resources[dev->resource_count].kind = DEVICE_RESOURCE_IOPORT;
      dev->resources[dev->resource_count].base = (u64)(raw & ~0x3U);
      dev->resources[dev->resource_count].size = 0;
      dev->resources[dev->resource_count].flags = 0;
      dev->resource_count += 1ULL;
    } else {
      dev->resources[dev->resource_count].kind = DEVICE_RESOURCE_MMIO;
      dev->resources[dev->resource_count].base = (u64)(raw & ~0xFULL);
      dev->resources[dev->resource_count].size = 0;
      dev->resources[dev->resource_count].flags = ((raw & 0x8U) != 0U) ? 1ULL : 0ULL;
      dev->resource_count += 1ULL;
    }
  }
}

static status_t enumerate_x86_legacy(void) {
  u64 bus;
  status_t st;

  st = ensure_pci_bus();
  if (st != STATUS_OK) {
    return st;
  }

  g_pci_device_count = 0;

  for (bus = 0; bus < 256ULL; ++bus) {
    u64 slot;
    for (slot = 0; slot < 32ULL; ++slot) {
      u64 fn_limit = 1ULL;
      u64 func;

      if ((header_type(bus, slot, 0) & 0x80ULL) != 0ULL) {
        fn_limit = 8ULL;
      }

      for (func = 0; func < fn_limit; ++func) {
        u64 vendor = read_vendor(bus, slot, func);
        device_t dev;
        unsigned int idreg;
        unsigned int classreg;
        u64 hdr;

        if (vendor == 0xFFFFULL) {
          continue;
        }

        idreg = pci_cfg_read32(bus, slot, func, 0x00U);
        classreg = pci_cfg_read32(bus, slot, func, 0x08U);
        hdr = header_type(bus, slot, func);

        dev.id = DEVICE_BUS_ID_NONE;
        dev.parent_id = DEVICE_BUS_ID_NONE;
        dev.bus_id = g_pci_bus_id;
        dev.name = "pci-device";
        dev.class_id = DEVICE_CLASS_PCI_DEVICE;
        dev.vendor_id = (u64)(idreg & 0xFFFFU);
        dev.device_id = (u64)((idreg >> 16) & 0xFFFFU);
        dev.revision = (u64)(classreg & 0xFFU);
        dev.class_code = (u64)((classreg >> 24) & 0xFFU);
        dev.subclass_code = (u64)((classreg >> 16) & 0xFFU);
        dev.prog_if = (u64)((classreg >> 8) & 0xFFU);
        dev.resource_count = 0;
        dev.driver_data = (void *)0;
        {
          u64 r;
          for (r = 0; r < DEVICE_BUS_MAX_RESOURCES; ++r) {
            dev.resources[r].kind = DEVICE_RESOURCE_NONE;
            dev.resources[r].base = 0;
            dev.resources[r].size = 0;
            dev.resources[r].flags = 0;
          }
        }
        fill_bar_resources(&dev, bus, slot, func, hdr);

        st = device_bus_register_device(&dev, (u64 *)0);
        if (st != STATUS_OK) {
          return st;
        }
        g_pci_device_count += 1ULL;
      }
    }
  }

  kprintf("pci: enumerated=%llu (x86 cfg io)\n", g_pci_device_count);
  return STATUS_OK;
}
#endif

status_t pci_enumerate(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

#if defined(__x86_64__)
  if (boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_NOT_SUPPORTED;
  }
  return enumerate_x86_legacy();
#else
  (void)boot_info;
  g_pci_device_count = 0;
  return STATUS_OK;
#endif
}

u64 pci_device_count(void) { return g_pci_device_count; }
