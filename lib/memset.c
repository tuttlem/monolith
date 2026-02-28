#include <stddef.h>

void *memset(void *dst, int value, size_t count) {
  unsigned char *ptr = (unsigned char *)dst;
  unsigned char byte = (unsigned char)value;

  for (size_t i = 0; i < count; ++i) {
    ptr[i] = byte;
  }

  return dst;
}
