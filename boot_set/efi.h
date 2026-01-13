#ifndef EFI_H
#define EFI_H

#include <stdint.h>
#include <stddef.h>

// ------------------ Basic Types ------------------
#define IN
#define OUT
#define OPTIONAL
#define CONST const

typedef uint8_t  UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint64_t UINTN;
typedef uint16_t CHAR16; // UCS-2
typedef uint8_t CHAR8;
typedef void    VOID;

typedef UINTN EFI_STATUS;
typedef VOID*  EFI_HANDLE;
typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;
typedef VOID *EFI_EVENT;
typedef UINTN EFI_TPL;

#define EVT_TIMER                            0x80000000
#define EVT_RUNTIME                          0x40000000
#define EVT_NOTIFY_WAIT                      0x00000100
#define EVT_NOTIFY_SIGNAL                    0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES        0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE    0x60000202

#define TPL_APPLICATION    4
#define TPL_CALLBACK       8
#define TPL_NOTIFY         16
#define TPL_HIGH_LEVEL     31

//#define EFI_PAGE_SIZE   SIZE_4KB
#define EFI_PAGE_MASK   0xFFF
#define EFI_PAGE_SHIFT  12

#define EFI_SUCCESS 0
#define EFIAPI __attribute__((ms_abi))
#define EFI_ERROR(Status) (((EFI_STATUS)(Status)) != EFI_SUCCESS)
#define EFI_ERROR_BIT 0x8000000000000000
#define EFI_STATUS_CODE(s) ((s) & ~EFI_ERROR_BIT)
#define EFI_SIZE_TO_PAGES(Size)  (((Size) >> EFI_PAGE_SHIFT) + (((Size) & EFI_PAGE_MASK) ? 1 : 0))

#define EFI_LOAD_ERROR 1
#define EFI_INVALID_PARAMETER 2
#define EFI_UNSUPPORTED 3
#define EFI_BAD_BUFFER_SIZE 4
#define EFI_BUFFER_TOO_SMALL 5
#define EFI_NOT_READY 6
#define EFI_DEVICE_ERROR 7
#define EFI_WRITE_PROTECTED 8
#define EFI_OUT_OF_RESOURCES 9
#define EFI_VOLUME_CORRUPTED 10
#define EFI_VOLUME_FULL 11
#define EFI_NO_MEDIA 12
#define EFI_MEDIA_CHANGED 13
#define EFI_NOT_FOUND 14
#define EFI_ACCESS_DENIED 15
#define EFI_NO_RESPONSE 16
#define EFI_NO_MAPPING 17
#define EFI_TIMEOUT 18
#define EFI_NOT_STARTED 19
#define EFI_ALREADY_STARTED 20
#define EFI_ABORTED 21
#define EFI_ICMP_ERROR 22
#define EFI_TFTP_ERROR 23
#define EFI_PROTOCOL_ERROR 24
#define EFI_INCOMPATIBLE_VERSION 25
#define EFI_SECURITY_VIOLATION 26
#define EFI_CRC_ERROR 27
#define EFI_END_OF_MEDIA 28
#define EFI_END_OF_FILE 31
#define EFI_INVALID_LANGUAGE 32
#define EFI_COMPROMISED_DATA 33
#define EFI_IP_ADDRESS_CONFLICT 34
#define EFI_HTTP_ERROR 35


// ------------------ EFI GUID ------------------
typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

// Forward declarations
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct _EFI_PCI_IO_PROTOCOL EFI_PCI_IO_PROTOCOL;

// ------------------ Locate Search Type ------------------
typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

// ------------------ Table Header ------------------
typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// ------------------ Simple Text Input ------------------
typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    OUT EFI_INPUT_KEY *Key
);

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    void* Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    void* WaitForKey;
};

// ------------------ Simple Text Output ------------------
typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN CHAR16 *String
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN UINTN Attribute
);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void* Reset;
    EFI_TEXT_STRING OutputString;
    void* TestString;
    void* QueryMode;
    void* SetMode;
    EFI_TEXT_SET_ATTRIBUTE SetAttribute;
    EFI_TEXT_CLEAR_SCREEN ClearScreen;
    void* SetCursorPosition;
    void* EnableCursor;
    void* Mode;
};

// ------------------ Runtime Services ------------------
typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef VOID (EFIAPI *EFI_RESET_SYSTEM)(
    IN EFI_RESET_TYPE ResetType,
    IN EFI_STATUS ResetStatus,
    IN UINTN DataSize,
    IN VOID* ResetData OPTIONAL
);

typedef struct {
    void* GetTime;
    void* SetTime;
    void* GetWakeupTime;
    void* SetWakeupTime;
    void* SetVirtualAddressMap;
    void* ConvertPointer;
    void* GetVariable;
    void* GetNextVariableName;
    void* SetVariable;
    void* GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM ResetSystem;
    void* UpdateCapsule;
    void* QueryCapsuleCapabilities;
    void* QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

// ------------------ Boot Services ------------------
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER)(
    IN EFI_LOCATE_SEARCH_TYPE SearchType,
    IN EFI_GUID *Protocol OPTIONAL,
    IN VOID *SearchKey OPTIONAL,
    OUT UINTN *NoHandles,
    OUT EFI_HANDLE **Buffer
);

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct {
   UINT32                     Type;
   EFI_PHYSICAL_ADDRESS       PhysicalStart;
   EFI_VIRTUAL_ADDRESS        VirtualStart;
   UINT64                     NumberOfPages;
   UINT64                     Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef enum {
   AllocateAnyPages,
   AllocateMaxAddress,
   AllocateAddress,
   MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef
EFI_STATUS
(EFIAPI  *EFI_ALLOCATE_POOL) (
   IN EFI_MEMORY_TYPE            PoolType,
   IN UINTN                      Size,
   OUT VOID                      **Buffer
);

typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_PAGES) (
   IN EFI_ALLOCATE_TYPE                   Type,
   IN EFI_MEMORY_TYPE                     MemoryType,
   IN UINTN                               Pages,
   IN OUT EFI_PHYSICAL_ADDRESS            *Memory
);

typedef
EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
  IN EFI_HANDLE                       ImageHandle,
  IN UINTN                            MapKey
);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
   IN OUT UINTN                  *MemoryMapSize,
   OUT EFI_MEMORY_DESCRIPTOR     *MemoryMap,
   OUT UINTN                     *MapKey,
   OUT UINTN                     *DescriptorSize,
   OUT UINT32                    *DescriptorVersion
);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL) (
  IN EFI_GUID                            *Protocol,
  IN VOID                                *Registration OPTIONAL,
  OUT VOID                               **Interface
);

typedef enum {
   EFI_NATIVE_INTERFACE
} EFI_INTERFACE_TYPE;

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (
   IN OUT EFI_HANDLE             *Handle,
   IN EFI_GUID                   *Protocol,
   IN EFI_INTERFACE_TYPE         InterfaceType,
   IN VOID                       *Interface
);

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (
   IN EFI_GUID                         *Protocol,
   IN EFI_EVENT                        Event,
   OUT VOID                            **Registration
);

typedef
VOID
(EFIAPI *EFI_EVENT_NOTIFY) (
  IN EFI_EVENT          Event,
  IN VOID              *Context
);

typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT) (
   IN UINT32                   Type,
   IN EFI_TPL                  NotifyTpl,
   IN EFI_EVENT_NOTIFY         NotifyFunction, OPTIONAL
   IN VOID                     *NotifyContext, OPTIONAL
   OUT EFI_EVENT               *Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_SIGNAL_EVENT) (
  IN EFI_EVENT Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
   IN UINTN             NumberOfEvents,
   IN EFI_EVENT         *Event,
   OUT UINTN            *Index
);

struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;

    void* RaiseTPL;
    void* RestoreTPL;

    EFI_ALLOCATE_PAGES AllocatePages;
    void* FreePages;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    EFI_STATUS (EFIAPI *FreePool)(VOID* Buffer);
    EFI_CREATE_EVENT CreateEvent;
    void* SetTimer;
    EFI_WAIT_FOR_EVENT WaitForEvent;
    EFI_SIGNAL_EVENT SignalEvent;
    void* CloseEvent;
    void* CheckEvent;

    EFI_INSTALL_PROTOCOL_INTERFACE InstallProtocolInterface;
    void* ReinstallProtocolInterface;
    void* UninstallProtocolInterface;
    EFI_STATUS (EFIAPI *HandleProtocol)(
        IN EFI_HANDLE Handle,
        IN EFI_GUID *Protocol,
        OUT VOID **Interface
    );
    void* Reserved;
    EFI_REGISTER_PROTOCOL_NOTIFY RegisterProtocolNotify;
    void* LocateHandle;
    void* LocateDevicePath;
    void* InstallConfigurationTable;

    void* LoadImage;
    void* StartImage;
    void* Exit;
    void* UnloadImage;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;

    void* GetNextMonotonicCount;
    void* Stall;
    void* SetWatchdogTimer;

    void* ConnectController;
    void* DisconnectController;

    void* OpenProtocol;
    void* CloseProtocol;
    void* OpenProtocolInformation;

    void* ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL LocateProtocol;
    void* InstallMultipleProtocolInterfaces;
    void* UninstallMultipleProtocolInterfaces;

    void* CalculateCrc32;
    void* CopyMem;
    void* SetMem;
    void* CreateEventEx;
};

// ------------------ System Table ------------------
typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16 *FirmwareVendor;
    UINT32 FirmwareRevision;
    void* ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    void* ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    void* StandardErrorHandle;
    void* StdErr;
    EFI_RUNTIME_SERVICES *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    UINTN NumberOfTableEntries;
    void* ConfigurationTable;
} EFI_SYSTEM_TABLE;

// ------------------ PCI IO ------------------
typedef enum {
    EfiPciIoWidthUint8,
    EfiPciIoWidthUint16,
    EfiPciIoWidthUint32
} EFI_PCI_IO_PROTOCOL_WIDTH;

#define EfiPciIoAttributeIo         0x00000001
#define EfiPciIoAttributeMemory     0x00000002
#define EfiPciIoAttributeBusMaster  0x00000004

typedef enum {
    EfiPciIoAttributeOperationGet,
    EfiPciIoAttributeOperationSet,
    EfiPciIoAttributeOperationEnable,
    EfiPciIoAttributeOperationDisable,
    EfiPciIoAttributeOperationSupported
} EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION;

typedef EFI_STATUS (EFIAPI *EFI_PCI_IO_PROTOCOL_ATTRIBUTES)(
    IN struct _EFI_PCI_IO_PROTOCOL *This,
    IN EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION Operation,
    IN UINT64 Attributes,
    OUT UINT64 *Result OPTIONAL
);

typedef EFI_STATUS (EFIAPI *EFI_PCI_IO_PROTOCOL_CONFIG_READ)(
    IN EFI_PCI_IO_PROTOCOL *This,
    IN EFI_PCI_IO_PROTOCOL_WIDTH Width,
    IN UINT32 Offset,
    IN UINTN Count,
    OUT VOID *Buffer
);

typedef struct {
    EFI_PCI_IO_PROTOCOL_CONFIG_READ Read;
    VOID *Write;
} EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS;

struct _EFI_PCI_IO_PROTOCOL {
    VOID* PollMem;
    VOID* PollIo;
    VOID* Mem;
    VOID* Io;
    EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS Pci;
    VOID* CopyMem;
    VOID* Map;
    VOID* Unmap;
    VOID* AllocateBuffer;
    VOID* FreeBuffer;
    VOID* Flush;
    VOID* GetLocation;
    EFI_PCI_IO_PROTOCOL_ATTRIBUTES Attributes;
    VOID* GetBarAttributes;
    VOID* SetBarAttributes;
    UINT64 RomSize;
    VOID* RomImage;
};

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
 {0x9042a9de,0x23dc,0x4a38,\
  {0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}}
// ----------------- GOP -------------------

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL;
typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32              RedMask;
    UINT32              GreenMask;
    UINT32              BlueMask;
    UINT32              ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
    UINT32                    Version;
    UINT32                    HorizontalResolution;
    UINT32                    VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
    EFI_PIXEL_BITMASK         PixelInformation;
    UINT32                    PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32                                    MaxMode;
    UINT32                                    Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION      *Info;
    UINTN                                      SizeOfInfo;
    EFI_PHYSICAL_ADDRESS                      FrameBufferBase;
    UINTN                                     FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE) (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL              *This,
    IN UINT32                                    ModeNumber,
    OUT UINTN                                    *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION     **Info
);

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE) (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL                *This,
    IN UINT32                                      ModeNumber
);

 typedef struct {
    UINT8                        Blue;
    UINT8                        Green;
    UINT8                        Red;
    UINT8                        Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef enum {
    EfiBltVideoFill,
    EfiBltVideoToBltBuffer,
    EfiBltBufferToVideo,
    EfiBltVideoToVideo,
    EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL                 *This,
    IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL            *BltBuffer, OPTIONAL
    IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION            BltOperation,
    IN UINTN                                        SourceX,
    IN UINTN                                        SourceY,
    IN UINTN                                        DestinationX,
    IN UINTN                                        DestinationY,
    IN UINTN                                        Width,
    IN UINTN                                        Height,
    IN UINTN                                        Delta OPTIONAL
);

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE     QueryMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE       SetMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT            Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE           *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;
// ------------------------- FILE SYS ------------------------
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000
#define EFI_FILE_MODE_READ       0x0000000000000001
#define EFI_FILE_MODE_WRITE      0x0000000000000002
#define EFI_FILE_MODE_CREATE     0x8000000000000000

#define EFI_FILE_READ_ONLY       0x0000000000000001
#define EFI_FILE_HIDDEN          0x0000000000000002
#define EFI_FILE_SYSTEM          0x0000000000000004
#define EFI_FILE_RESERVED        0x0000000000000008
#define EFI_FILE_DIRECTORY       0x0000000000000010
#define EFI_FILE_ARCHIVE         0x0000000000000020
#define EFI_FILE_VALID_ATTR      0x0000000000000037

struct _EFI_FILE_PROTOCOL;
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

typedef struct {
  UINT64                         Size;
  UINT64                         FileSize;
  UINT64                         PhysicalSize;
  VOID*                          CreateTime;
  VOID*                          LastAccessTime;
  VOID*                          ModificationTime;
  UINT64                         Attribute;
  CHAR16                         FileName [];
} EFI_FILE_INFO;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
  IN EFI_FILE_PROTOCOL                  *This,
  OUT EFI_FILE_PROTOCOL                 **NewHandle,
  IN CHAR16                             *FileName,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_CLOSE) (
  IN EFI_FILE_PROTOCOL                     *This
);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
  IN EFI_FILE_PROTOCOL           *This,
  IN OUT UINTN                   *BufferSize,
  OUT VOID                       *Buffer
);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_INFO) (
  IN EFI_FILE_PROTOCOL             *This,
  IN EFI_GUID                      *InformationType,
  IN OUT UINTN                     *BufferSize,
  OUT VOID                         *Buffer
);

typedef struct _EFI_FILE_PROTOCOL {
  UINT64                       Revision;
  EFI_FILE_OPEN                    Open;
  EFI_FILE_CLOSE                  Close;
  VOID*                          Delete;
  EFI_FILE_READ                    Read;
  VOID*                           Write;
  VOID*                     GetPosition;
  VOID*                     SetPosition;
  EFI_FILE_GET_INFO             GetInfo;
  VOID*                         SetInfo;
  VOID*                           Flush;
  VOID*                          OpenEx; // Added for revision 2
  VOID*                          ReadEx; // Added for revision 2
  VOID*                         WriteEx; // Added for revision 2
  VOID*                         FlushEx; // Added for revision 2
} EFI_FILE_PROTOCOL;

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL                   *This,
  OUT EFI_FILE_PROTOCOL                                **Root
);

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
 UINT64                                         Revision;
 EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME    OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
 {0x0964E5B2, 0x6459, 0x11D2, {0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}}
#define EFI_FILE_INFO_ID \
 {0x09576e92,0x6d3f,0x11d2, {0x8e39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

// ------------------ IMAGE LOAD --------------------------

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
  UINT8           Type;
  UINT8           SubType;
  UINT8           Length[2];
 } EFI_DEVICE_PATH_PROTOCOL;

 typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
   IN EFI_HANDLE           ImageHandle
);

typedef struct {
   UINT32                        Revision;
   EFI_HANDLE                    ParentHandle;
   EFI_SYSTEM_TABLE              *SystemTable;

   // Source location of the image
   EFI_HANDLE                    DeviceHandle;
   EFI_DEVICE_PATH_PROTOCOL      *FilePath;
   VOID                          *Reserved;

   // Image’s load options
   UINT32                        LoadOptionsSize;
   VOID                          *LoadOptions;

   // Location where image was loaded
   VOID                          *ImageBase;
   UINT64                        ImageSize;
   EFI_MEMORY_TYPE               ImageCodeType;
   EFI_MEMORY_TYPE               ImageDataType;
   EFI_IMAGE_UNLOAD              Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \ 
    {0x5B1B31A1,0x9562,0x11d2,\ 
        {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}}
#define EFI_DEVICE_PATH_PROTOCOL_GUID  \
    {0x09576e91,0x6d3f,0x11d2,\ 
        {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

// ------------------ RAM -------------------
typedef struct {
  UINT32                       Version ;
  UINT32                       NumberOfEntries ;
  UINT32                       DescriptorSize ;
  UINT32                       Flags ;
  // EFI_MEMORY_DESCRIPTOR     Entry [1] ;
}  EFI_MEMORY_ATTRIBUTES_TABLE;

#define EFI_MEMORY_ATTRIBUTES_TABLE_GUID \
  {0xdcfa911d, 0x26eb, 0x469f, \
    {0xa2, 0x20, 0x38, 0xb7, 0xdc, 0x46, 0x12, 0x20}}
// -------------------- MONITOR --------------------

typedef struct {
    UINT32                              SizeOfEdid;
    UINT8                               *Edid;
} EFI_EDID_DISCOVERED_PROTOCOL;

#define EFI_EDID_DISCOVERED_PROTOCOL_GUID \
 {0x1c0c34f6,0xd380,0x41fa,\
  {0xa0,0x49,0x8a,0xd0,0x6c,0x1a,0x66,0xaa}}

  #endif // EFI_H
