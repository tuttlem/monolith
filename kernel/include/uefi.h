#ifndef KERNEL_UEFI_H
#define KERNEL_UEFI_H

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;
#if defined(__x86_64__) || defined(__aarch64__)
typedef unsigned long long UINTN;
#else
typedef unsigned long UINTN;
#endif
typedef UINT16 CHAR16;
typedef void VOID;
typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;
typedef VOID *EFI_EVENT;

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

typedef struct {
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT8 Data4[8];
} EFI_GUID;

typedef struct {
  EFI_GUID VendorGuid;
  VOID *VendorTable;
} EFI_CONFIGURATION_TABLE;

typedef struct {
  UINT32 Type;
  UINT32 Pad;
  UINT64 PhysicalStart;
  UINT64 VirtualStart;
  UINT64 NumberOfPages;
  UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef struct {
  UINT32 RedMask;
  UINT32 GreenMask;
  UINT32 BlueMask;
  UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
  UINT32 Version;
  UINT32 HorizontalResolution;
  UINT32 VerticalResolution;
  UINT32 PixelFormat;
  EFI_PIXEL_BITMASK PixelInformation;
  UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
  UINT32 MaxMode;
  UINT32 Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN SizeOfInfo;
  UINT64 FrameBufferBase;
  UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

struct EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
  VOID *QueryMode;
  VOID *SetMode;
  VOID *Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

struct EFI_MP_SERVICES_PROTOCOL;
typedef void(EFIAPI *EFI_AP_PROCEDURE)(VOID *Buffer);
typedef EFI_STATUS(EFIAPI *EFI_MP_GET_NUMBER_OF_PROCESSORS)(struct EFI_MP_SERVICES_PROTOCOL *, UINTN *, UINTN *);
typedef EFI_STATUS(EFIAPI *EFI_MP_STARTUP_ALL_APS)(struct EFI_MP_SERVICES_PROTOCOL *, EFI_AP_PROCEDURE, UINT8, EFI_EVENT,
                                                    UINTN, VOID *, UINTN **);
typedef EFI_STATUS(EFIAPI *EFI_MP_WHOAMI)(struct EFI_MP_SERVICES_PROTOCOL *, UINTN *);

typedef struct EFI_MP_SERVICES_PROTOCOL {
  EFI_MP_GET_NUMBER_OF_PROCESSORS GetNumberOfProcessors;
  VOID *GetProcessorInfo;
  EFI_MP_STARTUP_ALL_APS StartupAllAPs;
  VOID *StartupThisAP;
  VOID *SwitchBSP;
  VOID *EnableDisableAP;
  EFI_MP_WHOAMI WhoAmI;
} EFI_MP_SERVICES_PROTOCOL;

typedef EFI_STATUS(EFIAPI *EFI_GET_MEMORY_MAP)(UINTN *, EFI_MEMORY_DESCRIPTOR *, UINTN *, UINTN *,
                                                UINT32 *);
typedef EFI_STATUS(EFIAPI *EFI_LOCATE_PROTOCOL)(EFI_GUID *, VOID *, VOID **);
typedef EFI_STATUS(EFIAPI *EFI_ALLOCATE_POOL)(UINT32, UINTN, VOID **);
typedef EFI_STATUS(EFIAPI *EFI_FREE_POOL)(VOID *);

typedef struct {
  EFI_TABLE_HEADER Hdr;
  VOID *RaiseTPL;
  VOID *RestoreTPL;
  VOID *AllocatePages;
  VOID *FreePages;
  EFI_GET_MEMORY_MAP GetMemoryMap;
  EFI_ALLOCATE_POOL AllocatePool;
  EFI_FREE_POOL FreePool;
  VOID *CreateEvent;
  VOID *SetTimer;
  VOID *WaitForEvent;
  VOID *SignalEvent;
  VOID *CloseEvent;
  VOID *CheckEvent;
  VOID *InstallProtocolInterface;
  VOID *ReinstallProtocolInterface;
  VOID *UninstallProtocolInterface;
  VOID *HandleProtocol;
  VOID *Reserved;
  VOID *RegisterProtocolNotify;
  VOID *LocateHandle;
  VOID *LocateDevicePath;
  VOID *InstallConfigurationTable;
  VOID *LoadImage;
  VOID *StartImage;
  VOID *Exit;
  VOID *UnloadImage;
  VOID *ExitBootServices;
  VOID *GetNextMonotonicCount;
  VOID *Stall;
  VOID *SetWatchdogTimer;
  VOID *ConnectController;
  VOID *DisconnectController;
  VOID *OpenProtocol;
  VOID *CloseProtocol;
  VOID *OpenProtocolInformation;
  VOID *ProtocolsPerHandle;
  VOID *LocateHandleBuffer;
  EFI_LOCATE_PROTOCOL LocateProtocol;
  VOID *InstallMultipleProtocolInterfaces;
  VOID *UninstallMultipleProtocolInterfaces;
  VOID *CalculateCrc32;
  VOID *CopyMem;
  VOID *SetMem;
  VOID *CreateEventEx;
} EFI_BOOT_SERVICES;

#define EFI_SUCCESS ((EFI_STATUS)0)
#define EFI_ERROR_MASK ((EFI_STATUS)(1ULL << ((sizeof(EFI_STATUS) * 8) - 1)))
#define EFI_ERROR(Status) (((Status) & EFI_ERROR_MASK) != 0)
#define EFI_BUFFER_TOO_SMALL ((EFI_STATUS)(EFI_ERROR_MASK | 5ULL))

#define EfiLoaderData 4U

#endif
