#include "status.h"

const char *status_str(status_t status) {
  switch (status) {
  case STATUS_OK:
    return "ok";
  case STATUS_INVALID_ARG:
    return "invalid_arg";
  case STATUS_NOT_FOUND:
    return "not_found";
  case STATUS_NO_MEMORY:
    return "no_memory";
  case STATUS_NOT_SUPPORTED:
    return "not_supported";
  case STATUS_BUSY:
    return "busy";
  case STATUS_FAULT:
    return "fault";
  case STATUS_INTERNAL:
    return "internal";
  case STATUS_TRY_AGAIN:
    return "try_again";
  case STATUS_DEFERRED:
    return "deferred";
  default:
    return "unknown";
  }
}
