#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "kernel.h"

#define SYSCALL_TRANSPORT_ABI_VERSION 1ULL

#define SYSCALL_RANGE_SUBSTRATE_BASE 0x0000ULL
#define SYSCALL_RANGE_SUBSTRATE_LIMIT 0x00FFULL
#define SYSCALL_RANGE_OS_BASE 0x0100ULL
#define SYSCALL_RANGE_OS_LIMIT 0x7FFFULL
#define SYSCALL_RANGE_VENDOR_BASE 0x8000ULL
#define SYSCALL_RANGE_VENDOR_LIMIT 0xFFFFULL

#define SYSCALL_OP_ABI_INFO 0x0001ULL
#define SYSCALL_OP_DEBUG_LOG 0x0002ULL
#define SYSCALL_OP_TIME_NOW 0x0003ULL
#define SYSCALL_OP_INPUT_TRY_READ 0x0004ULL

#define SYSCALL_ABI_FEATURE_TRAP_ENTRY (1ULL << 0)

typedef struct {
  u64 abi_version;
  u64 op;
  u64 args[6];
  u64 arch_id;
  u64 flags;
} syscall_request_t;

typedef struct {
  status_t status;
  u64 value;
} syscall_response_t;

typedef status_t (*syscall_handler_t)(const syscall_request_t *req, syscall_response_t *resp);

void syscall_reset(void);
status_t syscall_init(const boot_info_t *boot_info);
status_t syscall_register(u64 op, syscall_handler_t handler, const char *owner);
status_t syscall_dispatch(const syscall_request_t *req, syscall_response_t *resp);
status_t syscall_invoke_kernel(u64 op, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4,
                               u64 arg5, syscall_response_t *resp);
status_t syscall_invoke_trap(u64 op, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4,
                             u64 arg5, syscall_response_t *resp);
const char *syscall_owner(u64 op);
const char *syscall_op_name(u64 op);
u64 syscall_abi_info_word(void);
int syscall_trap_entry_ready(void);
int syscall_trap_mailbox_active(void);
status_t syscall_trap_mailbox_consume(void);
status_t syscall_handle_user_trap(u64 op, u64 arg0, u64 arg1, u64 arg2, u64 arg3,
                                  u64 arg4, u64 arg5, u64 *out_ret);
void syscall_dump_table(void);

#endif
