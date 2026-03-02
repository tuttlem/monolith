#ifndef KERNEL_CAPABILITY_PROFILE_H
#define KERNEL_CAPABILITY_PROFILE_H

#include "config.h"
#include "device_bus.h"
#include "status.h"

const char *capability_profile_name(void);
int capability_domain_enabled(device_class_t class_id);
const char *capability_domain_name(device_class_t class_id);
status_t capability_domain_state(device_class_t class_id);
void capability_domain_dump_matrix(void);
void capability_profile_print(void);

#endif
