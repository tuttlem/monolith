#ifndef KERNEL_PERSONALITY_H
#define KERNEL_PERSONALITY_H

#include "kernel.h"

#define PERSONALITY_MAX 8U

typedef BOOT_U64 personality_id_t;

typedef struct {
  const void *image_base;
  BOOT_U64 image_size;
  BOOT_U64 flags;
} exec_image_t;

typedef struct {
  BOOT_U64 entry_pc;
  BOOT_U64 stack_ptr;
  status_t status;
} exec_result_t;

typedef struct {
  personality_id_t id;
  const char *name;
  status_t (*on_activate)(void);
  status_t (*on_exec)(const exec_image_t *img, exec_result_t *out);
} personality_ops_t;

status_t personality_register(const personality_ops_t *ops);
status_t personality_activate(personality_id_t id);
status_t personality_exec(const exec_image_t *img, exec_result_t *out);
personality_id_t personality_active_id(void);
const char *personality_active_name(void);

#endif
