#ifndef KERNEL_PRINT_H
#define KERNEL_PRINT_H

#include <stdarg.h>

int kvsnprintf(char *buf, unsigned long size, const char *fmt, va_list args);
int ksnprintf(char *buf, unsigned long size, const char *fmt, ...);
int kprintf(const char *fmt, ...);

#endif
