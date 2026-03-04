#include "arch_user_mode.h"
#include "arch_cpu.h"

extern void x86_64_interrupts_rebind_code_selector(void);

typedef struct {
  unsigned short limit;
  BOOT_U64 base;
} __attribute__((packed)) x86_64_gdtr_t;

typedef struct {
  BOOT_U32 reserved0;
  BOOT_U64 rsp0;
  BOOT_U64 rsp1;
  BOOT_U64 rsp2;
  BOOT_U64 reserved1;
  BOOT_U64 ist1;
  BOOT_U64 ist2;
  BOOT_U64 ist3;
  BOOT_U64 ist4;
  BOOT_U64 ist5;
  BOOT_U64 ist6;
  BOOT_U64 ist7;
  BOOT_U64 reserved2;
  unsigned short reserved3;
  unsigned short iomap_base;
} __attribute__((packed)) x86_64_tss_t;

static BOOT_U64 g_x86_gdt[8] __attribute__((aligned(16)));
static x86_64_tss_t g_x86_tss __attribute__((aligned(16)));
static unsigned char g_x86_ring0_stack[16384] __attribute__((aligned(16)));
static BOOT_U64 g_x86_user_mode_ready;

#define X86_64_GDT_KERNEL_CODE_SEL 0x08U
#define X86_64_GDT_KERNEL_DATA_SEL 0x10U
#define X86_64_GDT_USER_CODE_SEL 0x1BU
#define X86_64_GDT_USER_DATA_SEL 0x23U
#define X86_64_GDT_TSS_SEL 0x28U

static void x86_lgdt(const x86_64_gdtr_t *gdtr) { __asm__ volatile("lgdt (%0)" : : "r"(gdtr) : "memory"); }
static void x86_ltr(unsigned short selector) { __asm__ volatile("ltr %0" : : "r"(selector) : "memory"); }

static void x86_load_segments(void) {
  __asm__ volatile("movw %[kds], %%ax\n\t"
                   "movw %%ax, %%ds\n\t"
                   "movw %%ax, %%es\n\t"
                   "movw %%ax, %%ss\n\t"
                   "pushq %[kcs]\n\t"
                   "leaq 1f(%%rip), %%rax\n\t"
                   "pushq %%rax\n\t"
                   "lretq\n\t"
                   "1:\n\t"
                   :
                   : [kds] "i"(X86_64_GDT_KERNEL_DATA_SEL), [kcs] "i"((BOOT_U64)X86_64_GDT_KERNEL_CODE_SEL)
                   : "rax", "memory");
}

static void x86_gdt_tss_init(void) {
  x86_64_gdtr_t gdtr;
  BOOT_U64 tss_base;
  BOOT_U64 tss_limit;

  g_x86_gdt[0] = 0ULL;
  g_x86_gdt[1] = 0x00AF9A000000FFFFULL; /* kernel code */
  g_x86_gdt[2] = 0x00AF92000000FFFFULL; /* kernel data */
  g_x86_gdt[3] = 0x00AFFA000000FFFFULL; /* user code */
  g_x86_gdt[4] = 0x00AFF2000000FFFFULL; /* user data */

  g_x86_tss.reserved0 = 0U;
  g_x86_tss.rsp0 = (BOOT_U64)(BOOT_UPTR)&g_x86_ring0_stack[sizeof(g_x86_ring0_stack)];
  g_x86_tss.rsp1 = 0ULL;
  g_x86_tss.rsp2 = 0ULL;
  g_x86_tss.reserved1 = 0ULL;
  g_x86_tss.ist1 = 0ULL;
  g_x86_tss.ist2 = 0ULL;
  g_x86_tss.ist3 = 0ULL;
  g_x86_tss.ist4 = 0ULL;
  g_x86_tss.ist5 = 0ULL;
  g_x86_tss.ist6 = 0ULL;
  g_x86_tss.ist7 = 0ULL;
  g_x86_tss.reserved2 = 0ULL;
  g_x86_tss.reserved3 = 0U;
  g_x86_tss.iomap_base = (unsigned short)sizeof(x86_64_tss_t);

  tss_base = (BOOT_U64)(BOOT_UPTR)&g_x86_tss;
  tss_limit = (BOOT_U64)sizeof(x86_64_tss_t) - 1ULL;
  g_x86_gdt[5] = (tss_limit & 0xFFFFULL) | ((tss_base & 0xFFFFFFULL) << 16) | (0x89ULL << 40) |
                 (((tss_limit >> 16) & 0xFULL) << 48) | (((tss_base >> 24) & 0xFFULL) << 56);
  g_x86_gdt[6] = (tss_base >> 32) & 0xFFFFFFFFULL;
  g_x86_gdt[7] = 0ULL;

  gdtr.limit = (unsigned short)(sizeof(g_x86_gdt) - 1U);
  gdtr.base = (BOOT_U64)(BOOT_UPTR)&g_x86_gdt[0];
  x86_lgdt(&gdtr);
  x86_load_segments();
  x86_ltr((unsigned short)X86_64_GDT_TSS_SEL);
  x86_64_interrupts_rebind_code_selector();
  g_x86_user_mode_ready = 1ULL;
}

status_t arch_user_mode_set_kernel_stack(void *kernel_stack_top) {
  if (g_x86_user_mode_ready == 0ULL) {
    x86_gdt_tss_init();
  }
  if (kernel_stack_top == (void *)0) {
    return STATUS_INVALID_ARG;
  }
  g_x86_tss.rsp0 = (BOOT_U64)(BOOT_UPTR)kernel_stack_top;
  return STATUS_OK;
}

status_t arch_user_mode_prepare_frame(arch_user_frame_t *frame) {
  if (frame == (arch_user_frame_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (frame->user_ip == 0ULL || frame->user_sp == 0ULL) {
    return STATUS_INVALID_ARG;
  }
  frame->user_sp &= ~0xFULL;
  return STATUS_OK;
}

__attribute__((noreturn)) void arch_user_mode_enter(arch_user_entry_t entry, void *arg, BOOT_U64 user_sp) {
  if (g_x86_user_mode_ready == 0ULL) {
    x86_gdt_tss_init();
  }
  if (entry == (arch_user_entry_t)0 || user_sp == 0ULL) {
    for (;;) {
      arch_cpu_halt();
    }
  }

  user_sp &= ~0xFULL;
  __asm__ volatile("movq %[arg], %%rdi\n\t"
                   "pushq %[uds]\n\t"
                   "pushq %[usp]\n\t"
                   "pushfq\n\t"
                   "andq $~0x200, (%%rsp)\n\t"
                   "pushq %[ucs]\n\t"
                   "pushq %[uip]\n\t"
                   "iretq\n\t"
                   :
                   : [arg] "r"((BOOT_U64)(BOOT_UPTR)arg), [uds] "i"((BOOT_U64)X86_64_GDT_USER_DATA_SEL),
                     [usp] "r"(user_sp), [ucs] "i"((BOOT_U64)X86_64_GDT_USER_CODE_SEL),
                     [uip] "r"((BOOT_U64)(BOOT_UPTR)entry)
                   : "rdi", "memory");
  __builtin_unreachable();
}
