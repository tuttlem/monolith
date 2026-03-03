#include "uaccess.h"

static BOOT_U64 g_user_base;
static BOOT_U64 g_user_size;
static BOOT_U64 g_user_window_active;

static int user_range_ok(BOOT_U64 p, BOOT_U64 n) {
  BOOT_U64 end;
  BOOT_U64 limit;

  if (g_user_window_active == 0ULL) {
    return 1;
  }
  if (n == 0ULL) {
    return 1;
  }
  if (p < g_user_base) {
    return 0;
  }
  end = p + n;
  if (end < p) {
    return 0;
  }
  limit = g_user_base + g_user_size;
  if (limit < g_user_base) {
    return 0;
  }
  if (end > limit) {
    return 0;
  }
  return 1;
}

status_t uaccess_set_user_window(BOOT_U64 base, BOOT_U64 size) {
  if (size == 0ULL) {
    g_user_window_active = 0ULL;
    g_user_base = 0ULL;
    g_user_size = 0ULL;
    return STATUS_OK;
  }
  if (base + size < base) {
    return STATUS_INVALID_ARG;
  }
  g_user_base = base;
  g_user_size = size;
  g_user_window_active = 1ULL;
  return STATUS_OK;
}

status_t copy_from_user_checked(void *dst, BOOT_U64 user_src, BOOT_U64 len) {
  BOOT_U64 i;
  const unsigned char *src;
  unsigned char *d;

  if (dst == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  if (!user_range_ok(user_src, len)) {
    return STATUS_FAULT;
  }

  src = (const unsigned char *)(BOOT_UPTR)user_src;
  d = (unsigned char *)dst;
  for (i = 0; i < len; ++i) {
    d[i] = src[i];
  }
  return STATUS_OK;
}

status_t copy_to_user_checked(BOOT_U64 user_dst, const void *src, BOOT_U64 len) {
  BOOT_U64 i;
  unsigned char *dst;
  const unsigned char *s;

  if (src == (const void *)0) {
    return STATUS_INVALID_ARG;
  }
  if (!user_range_ok(user_dst, len)) {
    return STATUS_FAULT;
  }

  dst = (unsigned char *)(BOOT_UPTR)user_dst;
  s = (const unsigned char *)src;
  for (i = 0; i < len; ++i) {
    dst[i] = s[i];
  }
  return STATUS_OK;
}
