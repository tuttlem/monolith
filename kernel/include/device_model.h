#ifndef KERNEL_DEVICE_MODEL_H
#define KERNEL_DEVICE_MODEL_H

#include "hw_desc.h"

typedef status_t (*driver_probe_fn)(const void *hw_node);
typedef status_t (*driver_init_fn)(void *dev);

typedef struct {
  const char *name;
  const char *class_name; /* irqc, timer, console, early */
  driver_probe_fn probe;
  driver_init_fn init;
} driver_t;

status_t driver_set_boot_info(const boot_info_t *boot_info);
void driver_registry_reset(void);
status_t driver_register(const driver_t *drv);
status_t driver_probe_all(const hw_desc_t *hw);
status_t driver_class_last_status(const char *class_name);

status_t device_model_register_builtin_drivers(void);

#endif
