#include "arch_interrupts.h"
#include "interrupts.h"

#define MIPS_STATUS_IE 0x00000001U
#define MIPS_STATUS_BEV 0x00400000U

static BOOT_U32 read_c0_status(void) {
  BOOT_U32 v;
  __asm__ volatile("mfc0 %0, $12" : "=r"(v));
  return v;
}

static void write_c0_status(BOOT_U32 v) {
  __asm__ volatile("mtc0 %0, $12\n\t"
                   "ehb"
                   :
                   : "r"(v)
                   : "memory");
}

status_t arch_interrupts_init(const boot_info_t *boot_info) {
  extern char mips_vector_base[];
  BOOT_U32 status;
  BOOT_U32 ebase;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_MIPS) {
    return STATUS_INVALID_ARG;
  }

  ebase = ((BOOT_U32)(BOOT_UPTR)mips_vector_base) & 0xFFFFF000U;
  __asm__ volatile("mtc0 %0, $15, 1\n\t"
                   "ehb"
                   :
                   : "r"(ebase)
                   : "memory");

  status = read_c0_status();
  status &= ~MIPS_STATUS_BEV;
  write_c0_status(status);
  return STATUS_OK;
}

void arch_interrupts_enable(void) {
  BOOT_U32 status = read_c0_status();
  write_c0_status(status | MIPS_STATUS_IE);
}

void arch_interrupts_disable(void) {
  BOOT_U32 status = read_c0_status();
  write_c0_status(status & ~MIPS_STATUS_IE);
}

BOOT_U32 mips_trap_c(BOOT_U32 cause, BOOT_U32 epc, BOOT_U32 badvaddr, BOOT_U32 status) {
  interrupt_frame_t frame;
  BOOT_U64 exc_code = (BOOT_U64)((cause >> 2) & 0x1FUL);
  BOOT_U64 vector = (exc_code == 0ULL) ? 32ULL : exc_code;

  if (exc_code == 0ULL) {
    /* PIC ACK/EOI is not wired yet; mask interrupts to avoid storms. */
    arch_interrupts_disable();
  }

  frame.arch_id = BOOT_INFO_ARCH_MIPS;
  frame.vector = vector;
  frame.error_code = (BOOT_U64)cause;
  frame.fault_addr = (BOOT_U64)badvaddr;
  frame.ip = (BOOT_U64)epc;
  frame.sp = 0;
  frame.flags = (BOOT_U64)status;
  interrupts_dispatch(&frame);
  return epc;
}
