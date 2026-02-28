#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t count) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  for (size_t i = 0; i < count; ++i) {
    d[i] = s[i];
  }

  return dst;
}
