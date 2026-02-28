#include "kernel.h"

typedef unsigned char u8;
typedef unsigned long u64;
typedef long s64;

#define OPAL_BUSY (-2L)
#define OPAL_SUCCESS 0L

extern s64 opal_console_write_call(u64 term, u64 *len, const u8 *buf);

static void opal_write(const u8 *buf, u64 len) {
  s64 rc;
  u64 written;
  u64 retries = 1000000;

  while (len > 0) {
    written = len;
    rc = opal_console_write_call(0, &written, buf);
    if (written > 0) {
      buf += written;
      len -= written;
      retries = 1000000;
      continue;
    }
    if (rc == OPAL_BUSY) {
      if (retries == 0) {
        return;
      }
      retries--;
      continue;
    }
    if (rc != OPAL_SUCCESS) {
      return;
    }
  }
}

void arch_puts(const char *s) {
  static const u8 crlf[2] = {'\r', '\n'};
  const char *start;

  while (*s != '\0') {
    start = s;
    while (*s != '\0' && *s != '\n') {
      s++;
    }
    if (s != start) {
      opal_write((const u8 *)start, (u64)(unsigned long)(s - start));
    }
    if (*s == '\n') {
      opal_write(crlf, 2);
      s++;
    }
  }
}

void arch_halt(void) { asm volatile("or 31,31,31"); }
