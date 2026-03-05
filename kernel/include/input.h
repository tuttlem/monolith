#ifndef KERNEL_INPUT_H
#define KERNEL_INPUT_H

#include "kernel.h"

status_t input_init(void);
status_t input_push_char(char ch);
status_t input_push_char_from_irq(char ch);
int input_try_pop_char(char *out_ch);
u64 input_drop_count(void);

#endif
