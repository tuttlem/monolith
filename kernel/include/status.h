#ifndef KERNEL_STATUS_H
#define KERNEL_STATUS_H

typedef int status_t;

#define STATUS_OK 0
#define STATUS_INVALID_ARG -1
#define STATUS_NOT_FOUND -2
#define STATUS_NO_MEMORY -3
#define STATUS_NOT_SUPPORTED -4
#define STATUS_BUSY -5
#define STATUS_FAULT -6
#define STATUS_INTERNAL -7
#define STATUS_TRY_AGAIN -8
#define STATUS_DEFERRED 1

static inline int status_is_ok(status_t status) { return status == STATUS_OK; }

const char *status_str(status_t status);

#endif
