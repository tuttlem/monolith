#include "kernel.h"
#include "print.h"

typedef unsigned long long u64_t;
#if __SIZEOF_POINTER__ == 8
typedef unsigned long long uptr_t;
#else
typedef unsigned long uptr_t;
#endif

static unsigned long append_char(char *buf, unsigned long size, unsigned long pos, char c) {
  if (size > 0 && pos + 1 < size) {
    buf[pos] = c;
  }
  return pos + 1;
}

static unsigned long append_str(char *buf, unsigned long size, unsigned long pos, const char *s) {
  if (s == (const char *)0) {
    s = "(null)";
  }
  while (*s != '\0') {
    pos = append_char(buf, size, pos, *s++);
  }
  return pos;
}

static unsigned long append_u32_dec(char *buf, unsigned long size, unsigned long pos, unsigned long value) {
  char tmp[32];
  unsigned i = 0;

  if (value == 0) {
    return append_char(buf, size, pos, '0');
  }

  while (value != 0 && i < (unsigned)sizeof(tmp)) {
    tmp[i++] = (char)('0' + (value % 10UL));
    value /= 10UL;
  }

  while (i > 0) {
    pos = append_char(buf, size, pos, tmp[--i]);
  }

  return pos;
}

static unsigned long append_u32_hex(char *buf, unsigned long size, unsigned long pos, unsigned long value,
                                    int lowercase) {
  char tmp[32];
  const char *digits = lowercase ? "0123456789abcdef" : "0123456789ABCDEF";
  unsigned i = 0;

  if (value == 0) {
    return append_char(buf, size, pos, '0');
  }

  while (value != 0 && i < (unsigned)sizeof(tmp)) {
    tmp[i++] = digits[value & 0xFUL];
    value >>= 4;
  }

  while (i > 0) {
    pos = append_char(buf, size, pos, tmp[--i]);
  }

  return pos;
}

static unsigned long append_u64_hex(char *buf, unsigned long size, unsigned long pos, u64_t value,
                                    int lowercase) {
  const char *digits = lowercase ? "0123456789abcdef" : "0123456789ABCDEF";
  int shift;
  int started = 0;

  for (shift = 60; shift >= 0; shift -= 4) {
    unsigned nibble = (unsigned)((value >> shift) & 0xFULL);
    if (!started) {
      if (nibble == 0 && shift != 0) {
        continue;
      }
      started = 1;
    }
    pos = append_char(buf, size, pos, digits[nibble]);
  }

  return pos;
}

int kvsnprintf(char *buf, unsigned long size, const char *fmt, va_list args) {
  unsigned long pos = 0;

  if (buf == (char *)0 || fmt == (const char *)0 || size == 0) {
    return 0;
  }

  while (*fmt != '\0') {
    int long_count = 0;

    if (*fmt != '%') {
      pos = append_char(buf, size, pos, *fmt++);
      continue;
    }

    ++fmt;
    if (*fmt == '%') {
      pos = append_char(buf, size, pos, '%');
      ++fmt;
      continue;
    }

    while (*fmt == 'l') {
      ++long_count;
      ++fmt;
    }

    switch (*fmt) {
    case 'c': {
      int v = va_arg(args, int);
      pos = append_char(buf, size, pos, (char)v);
      break;
    }
    case 's': {
      const char *s = va_arg(args, const char *);
      pos = append_str(buf, size, pos, s);
      break;
    }
    case 'd':
    case 'i': {
      long v;
      unsigned long mag;
      if (long_count >= 1) {
        v = va_arg(args, long);
      } else {
        v = (long)va_arg(args, int);
      }
      if (v < 0) {
        pos = append_char(buf, size, pos, '-');
        mag = (unsigned long)(-(v + 1L)) + 1UL;
      } else {
        mag = (unsigned long)v;
      }
      pos = append_u32_dec(buf, size, pos, mag);
      break;
    }
    case 'u':
      if (long_count >= 1) {
        pos = append_u32_dec(buf, size, pos, va_arg(args, unsigned long));
      } else {
        pos = append_u32_dec(buf, size, pos, (unsigned long)va_arg(args, unsigned int));
      }
      break;
    case 'x':
    case 'X': {
      int lower = (*fmt == 'x');
      if (long_count >= 2) {
        pos = append_u64_hex(buf, size, pos, va_arg(args, u64_t), lower);
      } else if (long_count == 1) {
        pos = append_u32_hex(buf, size, pos, va_arg(args, unsigned long), lower);
      } else {
        pos = append_u32_hex(buf, size, pos, (unsigned long)va_arg(args, unsigned int), lower);
      }
      break;
    }
    case 'p': {
      void *ptr = va_arg(args, void *);
      pos = append_str(buf, size, pos, "0x");
      if (__SIZEOF_POINTER__ == 8) {
        pos = append_u64_hex(buf, size, pos, (u64_t)(uptr_t)ptr, 1);
      } else {
        pos = append_u32_hex(buf, size, pos, (unsigned long)(uptr_t)ptr, 1);
      }
      break;
    }
    default:
      pos = append_char(buf, size, pos, '%');
      while (long_count-- > 0) {
        pos = append_char(buf, size, pos, 'l');
      }
      if (*fmt != '\0') {
        pos = append_char(buf, size, pos, *fmt);
      } else {
        --fmt;
      }
      break;
    }

    if (*fmt != '\0') {
      ++fmt;
    }
  }

  if (pos < size) {
    buf[pos] = '\0';
  } else {
    buf[size - 1] = '\0';
  }

  return (int)pos;
}

int ksnprintf(char *buf, unsigned long size, const char *fmt, ...) {
  va_list args;
  int written;

  va_start(args, fmt);
  written = kvsnprintf(buf, size, fmt, args);
  va_end(args);

  return written;
}

int kprintf(const char *fmt, ...) {
  char out[512];
  va_list args;
  int written;

  va_start(args, fmt);
  written = kvsnprintf(out, sizeof(out), fmt, args);
  va_end(args);

  arch_puts(out);
  return written;
}
