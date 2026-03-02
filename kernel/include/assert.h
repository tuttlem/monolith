#ifndef KERNEL_ASSERT_H
#define KERNEL_ASSERT_H

#include "panic.h"

#ifndef MONOLITH_ASSERT_ENABLE
#define MONOLITH_ASSERT_ENABLE 1
#endif

#ifndef MONOLITH_ASSERT_PANIC
#define MONOLITH_ASSERT_PANIC 1
#endif

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
