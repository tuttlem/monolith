#ifndef KERNEL_CAPABILITY_PROFILE_H
#define KERNEL_CAPABILITY_PROFILE_H

#include "config.h"
#include "device_bus.h"

const char *capability_profile_name(void);
int capability_domain_enabled(device_class_t class_id);
void capability_profile_print(void);

#endif
