#ifndef KERNEL_UEFI_H
#define KERNEL_UEFI_H

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;
typedef unsigned long UINTN;
typedef UINT16 CHAR16;
typedef void VOID;
typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;

#if defined(__x86_64__)
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI
#endif

typedef struct {
  UINT64 Signature;
  UINT32 Revision;
  UINT32 HeaderSize;
  UINT32 Crc32;
  UINT32 Reserved;
} EFI_TABLE_HEADER;

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS(EFIAPI *EFI_TEXT_RESET)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, UINT8);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_STRING)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, CHAR16 *);

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
  EFI_TEXT_RESET Reset;
  EFI_TEXT_STRING OutputString;
  VOID *TestString;
  VOID *QueryMode;
  VOID *SetMode;
  VOID *SetAttribute;
  VOID *ClearScreen;
  VOID *SetCursorPosition;
  VOID *EnableCursor;
  VOID *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
  EFI_TABLE_HEADER Hdr;
  CHAR16 *FirmwareVendor;
  UINT32 FirmwareRevision;
  EFI_HANDLE ConsoleInHandle;
  VOID *ConIn;
  EFI_HANDLE ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
  EFI_HANDLE StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
  VOID *RuntimeServices;
  VOID *BootServices;
  UINTN NumberOfTableEntries;
  VOID *ConfigurationTable;
} EFI_SYSTEM_TABLE;

#endif
