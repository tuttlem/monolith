#include "user_task.h"
#include "page_alloc.h"

#define USER_BOOTSTRAP_DEFAULT_BASE 0x40000000ULL

static status_t alloc_contiguous_pages(u64 pages, u64 *out_base) {
  u64 i;
  u64 first = 0ULL;
  u64 prev = 0ULL;
  u64 page_size = mm_page_size();

  if (pages == 0ULL || out_base == (u64 *)0) {
    return STATUS_INVALID_ARG;
  }

  for (i = 0; i < pages; ++i) {
    u64 page = alloc_page();
    if (page == 0ULL) {
      return STATUS_NO_MEMORY;
    }
    if (i == 0ULL) {
      first = page;
      prev = page;
      continue;
    }
    if (page != (prev + page_size)) {
      while (i != 0ULL) {
        --i;
        free_page(first + i * page_size);
      }
      free_page(page);
      return STATUS_TRY_AGAIN;
    }
    prev = page;
  }

  *out_base = first;
  return STATUS_OK;
}

status_t user_stack_alloc(u64 size, u64 *out_base) {
  u64 page_size = mm_page_size();
  u64 pages;

  if (size == 0ULL || out_base == (u64 *)0) {
    return STATUS_INVALID_ARG;
  }
  if ((size & (page_size - 1ULL)) != 0ULL) {
    return STATUS_INVALID_ARG;
  }

  pages = size / page_size;
  return alloc_contiguous_pages(pages, out_base);
}

status_t user_window_map(u64 base, u64 size, u64 prot) {
  status_t st;

  st = mm_protect(base, size, prot);
  if (st != STATUS_OK) {
    return st;
  }
  return mm_sync_tlb(base, size);
}

status_t user_task_bootstrap_prepare(const boot_info_t *boot_info, user_task_bootstrap_t *out_ctx) {
  u64 page_size = mm_page_size();
  status_t st;
  u64 stack_base;
  u64 user_base;

  if (boot_info == (const boot_info_t *)0 || out_ctx == (user_task_bootstrap_t *)0) {
    return STATUS_INVALID_ARG;
  }

  st = user_stack_alloc(page_size, &stack_base);
  if (st != STATUS_OK) {
    return st;
  }

  user_base = USER_BOOTSTRAP_DEFAULT_BASE;
  st = user_window_map(user_base, page_size, MMU_PROT_READ | MMU_PROT_WRITE | MMU_PROT_EXEC | MMU_PROT_USER);
  if (st != STATUS_OK) {
    free_page(stack_base);
    return st;
  }

  out_ctx->user_base = user_base;
  out_ctx->user_size = page_size;
  out_ctx->user_ip = user_base;
  out_ctx->user_sp = stack_base + page_size - 16ULL;
  out_ctx->kernel_stack_top = (void *)(uptr)boot_info->entry_sp;
  return STATUS_OK;
}
