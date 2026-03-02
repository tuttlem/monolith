#ifndef KERNEL_ASSERT_H
#define KERNEL_ASSERT_H

#include "config.h"
#include "panic.h"

#if MONOLITH_ASSERT_ENABLE
#if MONOLITH_ASSERT_PANIC
#define ASSERT(expr)                                                                                                      \
  do {                                                                                                                    \
    if (!(expr)) {                                                                                                        \
      panicf("assertion failed: %s (%s:%d)", #expr, __FILE__, __LINE__);                                                 \
    }                                                                                                                     \
  } while (0)
#else
#define ASSERT(expr) ((void)(expr))
#endif
#else
#define ASSERT(expr) ((void)(expr))
#endif

#endif
