#include "capability_profile.h"
#include "print.h"

const char *capability_domain_name(device_class_t class_id) {
  switch (class_id) {
  case DEVICE_CLASS_BLOCK:
    return "storage";
  case DEVICE_CLASS_NET:
    return "network";
  case DEVICE_CLASS_INPUT:
    return "input";
  case DEVICE_CLASS_DISPLAY:
    return "display";
  case DEVICE_CLASS_AUDIO:
    return "audio";
  default:
    return "core";
  }
}

const char *capability_profile_name(void) {
#if MONOLITH_CAP_PROFILE == MONOLITH_CAP_PROFILE_MINIMAL
  return "minimal";
#elif MONOLITH_CAP_PROFILE == MONOLITH_CAP_PROFILE_HEADLESS
  return "headless";
#elif MONOLITH_CAP_PROFILE == MONOLITH_CAP_PROFILE_DESKTOP
  return "desktop";
#else
  return "custom";
#endif
}

int capability_domain_enabled(device_class_t class_id) {
  switch (class_id) {
  case DEVICE_CLASS_BLOCK:
    return MONOLITH_CAP_STORAGE;
  case DEVICE_CLASS_NET:
    return MONOLITH_CAP_NETWORK;
  case DEVICE_CLASS_INPUT:
    return MONOLITH_CAP_INPUT;
  case DEVICE_CLASS_DISPLAY:
    return MONOLITH_CAP_DISPLAY;
  case DEVICE_CLASS_AUDIO:
    return MONOLITH_CAP_AUDIO;
  default:
    return 1;
  }
}

status_t capability_domain_state(device_class_t class_id) {
  return capability_domain_enabled(class_id) ? STATUS_OK : STATUS_DEFERRED;
}

void capability_domain_dump_matrix(void) {
  static const device_class_t domains[] = {DEVICE_CLASS_BLOCK, DEVICE_CLASS_NET, DEVICE_CLASS_INPUT,
                                           DEVICE_CLASS_DISPLAY, DEVICE_CLASS_AUDIO};
  BOOT_U64 i;
  kprintf("caps: matrix profile=%s\n", capability_profile_name());
  kprintf("  core: cpu=on mmu=on interrupts=on timer=on discovery=on reporting=on\n");
  for (i = 0; i < (BOOT_U64)(sizeof(domains) / sizeof(domains[0])); ++i) {
    status_t st = capability_domain_state(domains[i]);
    kprintf("  opt:%s=%s\n", capability_domain_name(domains[i]), status_str(st));
  }
}

void capability_profile_print(void) {
  kprintf("caps: profile=%s storage=%u network=%u input=%u display=%u audio=%u\n", capability_profile_name(),
          (unsigned)MONOLITH_CAP_STORAGE, (unsigned)MONOLITH_CAP_NETWORK, (unsigned)MONOLITH_CAP_INPUT,
          (unsigned)MONOLITH_CAP_DISPLAY, (unsigned)MONOLITH_CAP_AUDIO);
}
