#include "uefi.h"

static CHAR16 kHello[] = {
    'H', 'E', 'L', 'L', 'O', ' ', 'F', 'R', 'O', 'M', ' ', 'C', 'O', 'R', 'E',
    ' ', 'K', 'E', 'R', 'N', 'E', 'L', ' ', '(', 'U', 'E', 'F', 'I', ' ', 'x',
    '8', '6', '_', '6', '4', ')', '\r', '\n', 0};

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
  (void)image_handle;

  if (system_table == (VOID *)0 || system_table->ConOut == (VOID *)0 ||
      system_table->ConOut->OutputString == (VOID *)0) {
    return (EFI_STATUS)1;
  }

  system_table->ConOut->OutputString(system_table->ConOut, kHello);

  for (;;) {
  }
}
