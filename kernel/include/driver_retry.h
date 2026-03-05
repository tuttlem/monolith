#ifndef KERNEL_DRIVER_RETRY_H
#define KERNEL_DRIVER_RETRY_H

#include "arch_cpu.h"

typedef struct {
  u64 attempts;
  u64 max_attempts;
} driver_retry_t;

static inline void driver_retry_begin(driver_retry_t *ctx, u64 max_attempts) {
  if (ctx == (driver_retry_t *)0) {
    return;
  }
  ctx->attempts = 0ULL;
  ctx->max_attempts = max_attempts;
}

static inline int driver_retry_next(driver_retry_t *ctx) {
  if (ctx == (driver_retry_t *)0) {
    return 0;
  }
  if (ctx->attempts >= ctx->max_attempts) {
    return 0;
  }
  ctx->attempts += 1ULL;
  arch_cpu_relax();
  return 1;
}

#endif
