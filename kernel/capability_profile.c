#include "capability_profile.h"
#include "print.h"

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

void capability_profile_print(void) {
  kprintf("caps: profile=%s storage=%u network=%u input=%u display=%u audio=%u\n", capability_profile_name(),
          (unsigned)MONOLITH_CAP_STORAGE, (unsigned)MONOLITH_CAP_NETWORK, (unsigned)MONOLITH_CAP_INPUT,
          (unsigned)MONOLITH_CAP_DISPLAY, (unsigned)MONOLITH_CAP_AUDIO);
}
