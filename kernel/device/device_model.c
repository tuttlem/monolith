#include "device_model.h"
#include "device_bus.h"
#include "interrupts.h"
#include "print.h"
#include "timer.h"

#define DRIVER_MAX_COUNT 16U
#define CLASS_MAX_COUNT 4U
#define CLASS_IRQC "irqc"
#define CLASS_TIMER "timer"
#define CLASS_CONSOLE "console"
#define CLASS_EARLY "early"

typedef struct {
  const char *class_name;
  u64 index;
  const boot_info_t *boot_info;
  const hw_desc_t *hw;
  u64 device_id;
  const void *desc;
} device_node_t;

typedef struct {
  const char *class_name;
  status_t status;
} class_status_t;

static const boot_info_t *g_boot_info;
static const driver_t *g_drivers[DRIVER_MAX_COUNT];
static u64 g_driver_count;
static class_status_t g_class_status[CLASS_MAX_COUNT];

static u64 g_irq_driver_initialized;
static u64 g_timer_driver_initialized;
static u64 g_console_driver_initialized;

static int str_eq(const char *a, const char *b) {
  if (a == (const char *)0 || b == (const char *)0) {
    return 0;
  }
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static void class_status_reset(void) {
  g_class_status[0].class_name = CLASS_IRQC;
  g_class_status[0].status = STATUS_DEFERRED;
  g_class_status[1].class_name = CLASS_TIMER;
  g_class_status[1].status = STATUS_DEFERRED;
  g_class_status[2].class_name = CLASS_CONSOLE;
  g_class_status[2].status = STATUS_DEFERRED;
  g_class_status[3].class_name = CLASS_EARLY;
  g_class_status[3].status = STATUS_DEFERRED;
}

static void class_status_set(const char *class_name, status_t status) {
  u64 i;

  for (i = 0; i < CLASS_MAX_COUNT; ++i) {
    if (str_eq(g_class_status[i].class_name, class_name)) {
      g_class_status[i].status = status;
      return;
    }
  }
}

status_t driver_class_last_status(const char *class_name) {
  u64 i;

  for (i = 0; i < CLASS_MAX_COUNT; ++i) {
    if (str_eq(g_class_status[i].class_name, class_name)) {
      return g_class_status[i].status;
    }
  }
  return STATUS_NOT_FOUND;
}

status_t driver_set_boot_info(const boot_info_t *boot_info) {
  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  g_boot_info = boot_info;
  return STATUS_OK;
}

void driver_registry_reset(void) {
  u64 i;

  g_driver_count = 0;
  for (i = 0; i < DRIVER_MAX_COUNT; ++i) {
    g_drivers[i] = (const driver_t *)0;
  }
  class_status_reset();

  g_irq_driver_initialized = 0;
  g_timer_driver_initialized = 0;
  g_console_driver_initialized = 0;
}

status_t driver_register(const driver_t *drv) {
  if (drv == (const driver_t *)0 || drv->name == (const char *)0 || drv->class_name == (const char *)0 ||
      drv->probe == (driver_probe_fn)0 || drv->init == (driver_init_fn)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_driver_count >= DRIVER_MAX_COUNT) {
    return STATUS_NO_MEMORY;
  }

  g_drivers[g_driver_count] = drv;
  g_driver_count += 1ULL;
  return STATUS_OK;
}

static status_t run_driver_on_node(const driver_t *drv, const device_node_t *node) {
  status_t st_probe;
  status_t st_init;

  st_probe = drv->probe((const void *)node);
  if (st_probe != STATUS_OK) {
    if (st_probe != STATUS_DEFERRED && st_probe != STATUS_NOT_FOUND) {
      kprintf("device: probe fail class=%s drv=%s idx=%llu st=%s (%d)\n", node->class_name, drv->name, node->index,
              status_str(st_probe), st_probe);
    }
    return st_probe;
  }

  st_init = drv->init((void *)node);
  if (st_init != STATUS_OK && st_init != STATUS_DEFERRED) {
    kprintf("device: init fail class=%s drv=%s idx=%llu st=%s (%d)\n", node->class_name, drv->name, node->index,
            status_str(st_init), st_init);
  } else {
    kprintf("device: init class=%s drv=%s idx=%llu st=%s (%d)\n", node->class_name, drv->name, node->index,
            status_str(st_init), st_init);
  }
  return st_init;
}

static status_t probe_class(const char *class_name, const hw_desc_t *hw) {
  u64 i;
  device_class_t want = DEVICE_CLASS_UNKNOWN;
  status_t class_st = STATUS_DEFERRED;

  (void)hw;

  if (str_eq(class_name, CLASS_IRQC)) {
    want = DEVICE_CLASS_IRQC;
  } else if (str_eq(class_name, CLASS_TIMER)) {
    want = DEVICE_CLASS_TIMER;
  } else if (str_eq(class_name, CLASS_CONSOLE)) {
    want = DEVICE_CLASS_CONSOLE;
  } else if (str_eq(class_name, CLASS_EARLY)) {
    want = DEVICE_CLASS_MMIO;
  } else {
    class_status_set(class_name, STATUS_NOT_FOUND);
    return STATUS_NOT_FOUND;
  }

  for (i = 0; i < device_bus_count(); ++i) {
    const device_t *dev = device_bus_device_at(i);
    u64 j;

    if (dev == (const device_t *)0 || dev->class_id != want) {
      continue;
    }

    {
      device_node_t node;

      node.class_name = class_name;
      node.index = i;
      node.boot_info = g_boot_info;
      node.hw = (const hw_desc_t *)0;
      node.device_id = dev->id;
      node.desc = (const void *)dev;

      for (j = 0; j < g_driver_count; ++j) {
        const driver_t *drv = g_drivers[j];
        status_t st;
        if (!str_eq(drv->class_name, class_name)) {
          continue;
        }
        st = run_driver_on_node(drv, &node);
        if (st == STATUS_OK || st == STATUS_DEFERRED) {
          class_st = st;
          break;
        }
      }
    }
  }

  class_status_set(class_name, class_st);
  return class_st;
}

status_t driver_probe_all(const hw_desc_t *hw) {
  static const char *const init_order[] = {CLASS_IRQC, CLASS_TIMER, CLASS_CONSOLE, CLASS_EARLY};
  u64 i;
  status_t st = STATUS_OK;

  if (hw == (const hw_desc_t *)0 || g_boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  for (i = 0; i < (u64)(sizeof(init_order) / sizeof(init_order[0])); ++i) {
    status_t class_st = probe_class(init_order[i], hw);
    if (class_st != STATUS_OK && class_st != STATUS_DEFERRED) {
      st = class_st;
    }
  }

  return st;
}

static status_t irq_driver_probe(const void *hw_node) {
  const device_node_t *node = (const device_node_t *)hw_node;
  const device_t *dev;
  if (node == (const device_node_t *)0) {
    return STATUS_NOT_FOUND;
  }
  dev = (const device_t *)node->desc;
  if (dev == (const device_t *)0 || dev->class_id != DEVICE_CLASS_IRQC) {
    return STATUS_NOT_FOUND;
  }
  return STATUS_OK;
}

static status_t irq_driver_init(void *dev) {
  const device_node_t *node = (const device_node_t *)dev;
  if (node == (const device_node_t *)0 || node->boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_irq_driver_initialized != 0ULL) {
    return STATUS_OK;
  }
  g_irq_driver_initialized = 1ULL;
  return interrupts_init(node->boot_info);
}

static status_t timer_driver_probe(const void *hw_node) {
  const device_node_t *node = (const device_node_t *)hw_node;
  const device_t *dev;
  if (node == (const device_node_t *)0) {
    return STATUS_NOT_FOUND;
  }
  dev = (const device_t *)node->desc;
  if (dev == (const device_t *)0 || dev->class_id != DEVICE_CLASS_TIMER) {
    return STATUS_NOT_FOUND;
  }
  return STATUS_OK;
}

static status_t timer_driver_init(void *dev) {
  const device_node_t *node = (const device_node_t *)dev;
  if (node == (const device_node_t *)0 || node->boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_timer_driver_initialized != 0ULL) {
    return STATUS_OK;
  }
  g_timer_driver_initialized = 1ULL;
  return timer_init(node->boot_info);
}

static status_t console_driver_probe(const void *hw_node) {
  const device_node_t *node = (const device_node_t *)hw_node;
  const device_t *dev;
  if (node == (const device_node_t *)0) {
    return STATUS_INVALID_ARG;
  }
  dev = (const device_t *)node->desc;
  if (dev != (const device_t *)0 && (dev->class_id == DEVICE_CLASS_CONSOLE || dev->class_id == DEVICE_CLASS_FRAMEBUFFER)) {
    return STATUS_OK;
  }
  return STATUS_NOT_FOUND;
}

static status_t console_driver_init(void *dev) {
  const device_node_t *node = (const device_node_t *)dev;
  if (node == (const device_node_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_console_driver_initialized != 0ULL) {
    return STATUS_OK;
  }
  g_console_driver_initialized = 1ULL;
  return STATUS_OK;
}

status_t device_model_register_builtin_drivers(void) {
  static const driver_t irq_driver = {
      .name = "irq-backend",
      .class_name = CLASS_IRQC,
      .probe = irq_driver_probe,
      .init = irq_driver_init,
  };
  static const driver_t timer_driver = {
      .name = "timer-backend",
      .class_name = CLASS_TIMER,
      .probe = timer_driver_probe,
      .init = timer_driver_init,
  };
  static const driver_t console_driver = {
      .name = "early-console",
      .class_name = CLASS_CONSOLE,
      .probe = console_driver_probe,
      .init = console_driver_init,
  };
  status_t st;

  st = driver_register(&irq_driver);
  if (st != STATUS_OK) {
    return st;
  }
  st = driver_register(&timer_driver);
  if (st != STATUS_OK) {
    return st;
  }
  return driver_register(&console_driver);
}
