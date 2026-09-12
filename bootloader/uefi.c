#include "efi.h"
#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

#include "elf.h"
#include "../common/logo.h"
#include "../common/fromboot.h" // Contains after boot info for kernel
#include "../common/fonts/cozette.h"

#define PAGE_SIZE   0x1000
#define HUGE_PAGE   0x200000ULL

#define PTE_PRESENT  (1ULL << 0)
#define PTE_RW       (1ULL << 1)
#define PTE_USER     (1ULL << 2)
#define PTE_PS       (1ULL << 7)

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

typedef int (*KernelEntry)(BootInfo*);
//typedef void (*KernelHeader)(BootInfo*);

// FOR DEBUG PURPUSES ONLY -- REMOVE LATER
static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

void serial_putchar(char c)
{
    outb(0x3F8, c);
}

void serial_print(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        /*
        switch (str[i]) {
            case '\n':
                serial_putchar('\r');
                serial_putchar('\n');
                break;
            case '\t':
                serial_putchar(' ');
            default:
                serial_putchar(str[i]);
                break;
        }
        */
        serial_putchar(str[i]);
        i++;
    }
}

void serial_print_hex(uint64_t value)
{
    const char hex_chars[] = "0123456789ABCDEF";

    char buffer[19];

    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = '\0';

    for (int i = 17; i >= 2; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    serial_print(buffer);
    serial_putchar('\n');
}

// AI code -remove
#define SCALE 20

void draw_glyph(
    uint8_t* glyph,
    UINTN glyph_height,
    UINTN pos_x,
    UINTN pos_y,
    UINTN pitch,
    UINT32 *fb
)
{
    for (UINTN y = 0; y < glyph_height; y++)
    {
        uint8_t row = glyph[y];

        for (UINTN x = 0; x < 8; x++)
        {
            if (row & (0x80 >> x))
            {
                // scale each font pixel
                for (UINTN sy = 0; sy < SCALE; sy++)
                {
                    for (UINTN sx = 0; sx < SCALE; sx++)
                    {
                        fb[(pos_y + y*SCALE + sy) * pitch +
                           (pos_x + x*SCALE + sx)] = 0xFF0000;
                    }
                }
            }
        }
    }
}

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xppm;
    int32_t yppm;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} BMPInfoHeader;
#pragma pack(pop)
/*
VOID *CopyMem(
    VOID *Destination,
    CONST VOID *Source,
    UINTN Length
)
{
    UINT8 *dst = (UINT8 *)Destination;
    CONST UINT8 *src = (CONST UINT8 *)Source;

    for (UINTN i = 0; i < Length; i++)
    {
        dst[i] = src[i];
    }

    return Destination;
}
*/

void DrawBMP(UINT32 *fb, UINTN pitch, UINTN posX, UINTN posY, unsigned char *bmp, UINTN scale) {
    BMPHeader *header = (BMPHeader *)bmp;
    BMPInfoHeader *info = (BMPInfoHeader *)(bmp + sizeof(BMPHeader));
    if (header->type != 0x4D42)
        return;

    if (info->bpp != 24)
        return;

    unsigned char *pixels = bmp + header->offset;

    UINTN width = info->width;
    UINTN height = info->height;
    UINTN rowSize = (width * 3 + 3) & ~3;

    for (UINTN y = 0; y < height; y++) {
        for (UINTN x = 0; x < width; x++) {
            unsigned char *pixel =
                pixels + (height - 1 - y) * rowSize + x * 3;
            UINT32 color =
                ((UINT32)pixel[2] << 16) |
                ((UINT32)pixel[1] << 8)  |
                ((UINT32)pixel[0]);

            for (UINTN sy = 0; sy < scale; sy++) {
                for (UINTN sx = 0; sx < scale; sx++) {
                    fb[(posY + y * scale + sy) * pitch + (posX + x * scale + sx)] = color;
                }
            }
        }
    }
}

void uefi_print(EFI_SYSTEM_TABLE *st, const CHAR16 *str);
void uefi_println(EFI_SYSTEM_TABLE *st, const CHAR16 *str);
void uefi_status_print(EFI_SYSTEM_TABLE *st, EFI_STATUS stat);
void uefi_status_println(EFI_SYSTEM_TABLE *st, EFI_STATUS stat);

EFI_GUID gEfiPciIoProtocolGuid =
 {0x4cf5b200,0x68b8,0x4ca5,{0x9e,0xec,0xb2,0x3e,0x3f,0x50,0x02,0x9a}};

EFI_BOOT_SERVICES *gBS;

typedef struct {
    UINT16 id;
    const char *name;
} pci_vendor_t;

static const pci_vendor_t pci_vendors[] = {
    { 0x8086, "Intel"   },
    { 0x10DE, "NVIDIA"  },
    { 0x1002, "AMD"     },
    { 0x1022, "AMD"     }, // legacy
    { 0x1234, "QEMU VGA"},
    { 0x1AF4, "VirtIO"  },
    { 0x1B36, "Red Hat" },
};

typedef struct {
    const CHAR16 *string;
    int is;
} cpu_vendor_identifiers;

static const cpu_vendor_identifiers isCPUVMArray[] = { 
    // Its kinda simple but easly bypassed with hardware spoofing, but its mainly for cpu detections
    { L"AuthenticAMD", 0 },
    { L"AMDisbetter!", 0 }, // Early engineering samples of AMD K5 processor (probably doesnt support uefi)
    { L"GenuineIntel", 0 },
    { L"VIAVIAVIA", 0 },
    { L"GenuineTMx86", 0 },
    { L"TransmetaCPU", 0 },
    { L"CyrixInstead", 0 },
    { L"CentaurHauls", 0 },
    { L"NexGenDriven", 0 },
    { L"UMCUMCUMC", 0 },
    { L"SiSSiSSiS", 0 },
    { L"GeodebyNSC", 0 },
    { L"RiseRiseRise", 0 },
    { L"Vortex86SoC", 0 },
    { L"MiSTerAO486", 0 },
    { L"GenuineAO486", 0 },
    { L"Shanghai", 0 },
    { L"HygonGenuine", 0 },
    { L"E2K MACHINE ", 0 },
    
    // Vendor strings from hypervisors.
    { L"TCGTCGTCGTCG", 1 },
    { L"KVMKVMKVM", 1 },
    { L"VMwareVMware", 1 },
    { L"VBoxVBoxVBox", 1 },
    { L"XenVMMXenVMM", 1 },
    { L"MicrosoftHv", 1 },
    { L"prlhyperv", 1 },
    { L"lrpepyhvr", 1 },
    { L"bhyvebhyve", 1 },
    { L"QNXQVMBSQG", 1 },
};

int strcmp16(const CHAR16 *x, const CHAR16 *y) {
    while (*x && (*x == *y)) {
        x++;
        y++;
    }
    return (int)(*x - *y);
}

int isCPUVM(const CHAR16 *string) {
    int count = sizeof(isCPUVMArray) / sizeof(isCPUVMArray[0]);
    for (int i = 0; i < count; i++) {
        if (strcmp16(string, isCPUVMArray[i].string) == 0) {
            return isCPUVMArray[i].is;
        }
    }
    return 2; // unknown
}

// convert ASCII to UEFI CHAR16 string
void ascii_to_char16(const char *src, CHAR16 *dst, size_t max_len) {
    size_t i;
    for (i = 0; i < max_len-1 && src[i]; i++) {
        dst[i] = (CHAR16)src[i];
    }
    dst[i] = 0; // null-terminate
}

// convert UINTN to CHAR16 string
void decimalConvert(UINTN n, CHAR16 *buf, size_t buf_size) {
    size_t pos = 0;

    if(buf_size == 0) return;

    do {
        if(pos >= buf_size - 1) break;
        buf[pos++] = L'0' + (n % 10);
        n /= 10;
    } while(n);

    buf[pos] = 0;

    // reverse string
    for(size_t i = 0; i < pos / 2; i++) {
        CHAR16 t = buf[i];
        buf[i] = buf[pos - 1 - i];
        buf[pos - 1 - i] = t;
    }
}

// helper to print a line with CRLF
void uefi_println(EFI_SYSTEM_TABLE *st, const CHAR16 *str) {
    st->ConOut->OutputString(st->ConOut, str);
    st->ConOut->OutputString(st->ConOut, L"\r\n");
}

void uefi_print(EFI_SYSTEM_TABLE *st, const CHAR16 *str) {
    st->ConOut->OutputString(st->ConOut, str);
}

static inline void write_cr3(uint64_t value) {
    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r"(value)
        : "memory"
    );
}

static inline uint64_t read_rip(void) {
    uint64_t value;
    __asm__ volatile (
        "lea 0(%%rip), %0"
        : "=r"(value)
    );

    return value;
}
typedef struct {
    const CHAR16 *name;
    EFI_STATUS value;
} efi_stat_char16_struct;

efi_stat_char16_struct const efi_status_char16[] = {
    { L"EFI_LOAD_ERROR", EFI_LOAD_ERROR },
    { L"EFI_INVALID_PARAMETER", EFI_INVALID_PARAMETER },
    { L"EFI_UNSUPPORTED", EFI_UNSUPPORTED },
    { L"EFI_BAD_BUFFER_SIZE", EFI_BAD_BUFFER_SIZE },
    { L"EFI_BUFFER_TOO_SMALL", EFI_BUFFER_TOO_SMALL },
    { L"EFI_NOT_READY", EFI_NOT_READY },
    { L"EFI_DEVICE_ERROR", EFI_DEVICE_ERROR },
    { L"EFI_WRITE_PROTECTED", EFI_WRITE_PROTECTED },
    { L"EFI_OUT_OF_RESOURCES", EFI_OUT_OF_RESOURCES },
    { L"EFI_VOLUME_CORRUPTED", EFI_VOLUME_CORRUPTED },
    { L"EFI_VOLUME_FULL", EFI_VOLUME_FULL },
    { L"EFI_NO_MEDIA", EFI_NO_MEDIA },
    { L"EFI_MEDIA_CHANGED", EFI_MEDIA_CHANGED },
    { L"EFI_NOT_FOUND", EFI_NOT_FOUND },
    { L"EFI_ACCESS_DENIED", EFI_ACCESS_DENIED },
    { L"EFI_NO_RESPONSE", EFI_NO_RESPONSE },
    { L"EFI_NO_MAPPING", EFI_NO_MAPPING },
    { L"EFI_TIMEOUT", EFI_TIMEOUT },
    { L"EFI_NOT_STARTED", EFI_NOT_STARTED },
    { L"EFI_ALREADY_STARTED", EFI_ALREADY_STARTED },
    { L"EFI_ABORTED", EFI_ABORTED },
    { L"EFI_ICMP_ERROR", EFI_ICMP_ERROR },
    { L"EFI_TFTP_ERROR", EFI_TFTP_ERROR },
    { L"EFI_PROTOCOL_ERROR", EFI_PROTOCOL_ERROR },
    { L"EFI_INCOMPATIBLE_VERSION", EFI_INCOMPATIBLE_VERSION },
    { L"EFI_SECURITY_VIOLATION", EFI_SECURITY_VIOLATION },
    { L"EFI_CRC_ERROR", EFI_CRC_ERROR },
    { L"EFI_END_OF_MEDIA", EFI_END_OF_MEDIA },
    { L"EFI_END_OF_FILE", EFI_END_OF_FILE },
    { L"EFI_INVALID_LANGUAGE", EFI_INVALID_LANGUAGE },
    { L"EFI_COMPROMISED_DATA", EFI_COMPROMISED_DATA },
    { L"EFI_IP_ADDRESS_CONFLICT", EFI_IP_ADDRESS_CONFLICT },
    { L"EFI_HTTP_ERROR", EFI_HTTP_ERROR },
    // name, value
};

void uefi_status_println(EFI_SYSTEM_TABLE *st, EFI_STATUS stat) {
    EFI_STATUS code = stat;
    UINTN count = sizeof(efi_status_char16) / sizeof(efi_status_char16[0]);
    for (UINTN i = 0; i < count; i++) {
        EFI_STATUS value = efi_status_char16[i].value; // Because it is pointer, now its value!
        if (value == code) {
            st->ConOut->OutputString(st->ConOut, efi_status_char16[i].name);
            st->ConOut->OutputString(st->ConOut, L"\r\n");
            return;
        }
    }
    st->ConOut->OutputString(st->ConOut, L"EFI_STATUS_UNKNOWN: ");
    CHAR16 buf[32];
    decimalConvert(code, buf, 32);
    st->ConOut->OutputString(st->ConOut, buf);
    st->ConOut->OutputString(st->ConOut, L"\r\n");
}

void uefi_status_print(EFI_SYSTEM_TABLE *st, EFI_STATUS stat) {
    EFI_STATUS code = stat;
    UINTN count = sizeof(efi_status_char16) / sizeof(efi_status_char16[0]);
    for (UINTN i = 0; i < count; i++) {
        EFI_STATUS value = efi_status_char16[i].value; // Because it is pointer, now its value!
        if (value == code) {
            st->ConOut->OutputString(st->ConOut, efi_status_char16[i].name);
            return;
        }
    }
    st->ConOut->OutputString(st->ConOut, L"EFI_STATUS_UNKNOWN: ");
    CHAR16 buf[32];
    decimalConvert(code, buf, 32);
    st->ConOut->OutputString(st->ConOut, buf);
}

void uefi_print_hex(EFI_SYSTEM_TABLE *st, UINT64 value)
{
    const CHAR16 hex_chars[] = {
        '0','1','2','3','4','5','6','7',
        '8','9','A','B','C','D','E','F',
        0
    };

    CHAR16 buffer[19];

    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = 0;

    for (int i = 17; i >= 2; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    uefi_println(st, buffer);
}

typedef struct {
    const CHAR16 *name;
    EFI_GRAPHICS_PIXEL_FORMAT value;
} efi_pixel_map;

efi_pixel_map efi_pixel_map_array[] = {
    {L"PixelRedGreenBlueReserved8BitPerColor", PixelRedGreenBlueReserved8BitPerColor},
    {L"PixelBlueGreenRedReserved8BitPerColor", PixelBlueGreenRedReserved8BitPerColor},
    {L"PixelBitMask", PixelBitMask},
    {L"PixelBltOnly", PixelBltOnly},
    {L"PixelFormatMax", PixelFormatMax},
};

void printGOPStats(EFI_SYSTEM_TABLE *st, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION info) {
    UINT32 version = info.Version;
    UINT32 HorizontalResolution = info.HorizontalResolution;
    UINT32 VerticalResolution = info.VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT pixelFormat = info.PixelFormat;
    EFI_PIXEL_BITMASK PixelInformation = info.PixelInformation;
    UINT32 PixelsPerScanLine = info.PixelsPerScanLine;

    CHAR16 versionBuf[32];
    CHAR16 HorizontalResolutionBuf[32];
    CHAR16 VerticalResolutionBuf[32];
    CHAR16 PixelsPerScanLineBuf[128];

    decimalConvert(version, versionBuf, 32);
    decimalConvert(HorizontalResolution, HorizontalResolutionBuf, 32);
    decimalConvert(VerticalResolution, VerticalResolutionBuf, 32);
    decimalConvert(PixelsPerScanLine, PixelsPerScanLineBuf, 32);

    CHAR16 pixelFormatChar16 = L"NoneFound";
    for (UINTN i = 0; i < sizeof(efi_pixel_map_array)/sizeof(efi_pixel_map_array[0]); i++) {
        efi_pixel_map a = efi_pixel_map_array[i];
        if (a.value == pixelFormat)
            pixelFormatChar16 = a.name;
    }

    uefi_println(st, L"M--------------------");
    uefi_print(st, L"\tVersion             :");
    uefi_println(st, version);
    uefi_print(st, L"\tResolution          :");
    uefi_print(st, HorizontalResolutionBuf);
    uefi_print(st, L"x");
    uefi_println(st, VerticalResolutionBuf);
    uefi_print(st, L"\tPixel per scanline  :");
    uefi_println(st, PixelsPerScanLineBuf);
    uefi_print(st, L"\tPixel formation     :");
    uefi_println(st, pixelFormatChar16);
}

const char *pci_vendor_name(UINT16 id, int *status) {
    for (UINTN i = 0; i < sizeof(pci_vendors)/sizeof(pci_vendors[0]); i++) {
        if (pci_vendors[i].id == id)
            return pci_vendors[i].name;
    }
    *status = 1;
    return "Unknown";
}

void get_cpu_vendor(char vendor[13]) {
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(0, &eax, &ebx, &ecx, &edx);
    ((unsigned int*)vendor)[0] = ebx;
    ((unsigned int*)vendor)[1] = edx;
    ((unsigned int*)vendor)[2] = ecx;
    vendor[12] = 0;
}

void get_cpu_brand(char brand[49]) {
    unsigned int regs[4];
    for (unsigned int i = 0; i < 3; i++) {
        __get_cpuid(0x80000002 + i, &regs[0], &regs[1], &regs[2], &regs[3]);
        ((unsigned int*)brand)[i*4 + 0] = regs[0];
        ((unsigned int*)brand)[i*4 + 1] = regs[1];
        ((unsigned int*)brand)[i*4 + 2] = regs[2];
        ((unsigned int*)brand)[i*4 + 3] = regs[3];
    }
    brand[48] = 0;
}

void vendor_to_char16(const char vendor[13], CHAR16 vendor16[13]) {
    for (int i = 0; i < 12; i++) {
        vendor16[i] = (vendor[i] == 0 ? L' ' : (CHAR16)vendor[i]);
    }
    vendor16[12] = 0; // null terminate
}

EFI_EVENT gopEventCallProceed;

VOID EFIAPI GopInstalledCallback(
    IN EFI_EVENT Event,
    IN VOID      *Context
) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    VOID *GopRegistration = NULL;

    if (!EFI_ERROR(
        gBS->LocateProtocol(
            &gEfiGraphicsOutputProtocolGuid,
            GopRegistration,
            (VOID **)&Gop
        )
    )) {
        // Now its defined
        gBS->SignalEvent(gopEventCallProceed);
    }
}



EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    gBS = SystemTable->BootServices;
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

    uefi_println(SystemTable, L"[System] Booted, 1st stage...");
    uefi_println(SystemTable, L"[System] Machine stats:");
    uefi_println(SystemTable, L"-----------------------");
    uefi_print(SystemTable, L"Firmware Vendor: ");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, SystemTable->FirmwareVendor);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");

    unsigned int max_ext;
    __get_cpuid(0x80000000, &max_ext, NULL, NULL, NULL);

    uefi_println(SystemTable, L"----- CPU -----");
    CHAR16 vendor16[13];
    if (max_ext < 0x80000004) {
        uefi_println(SystemTable, L"CPUID is not supported.");
    } else {
        char vendor[13];
        get_cpu_vendor(vendor);
        vendor_to_char16(vendor, vendor16);

        uefi_print(SystemTable, L"CPU Vendor string: ");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, vendor16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");

        char brand[50];
        CHAR16 brand16[64];
        get_cpu_brand(brand);
        ascii_to_char16(brand, brand16, 64);

        uefi_print(SystemTable, L"CPU: ");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, brand16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");
    }

    int a = isCPUVM(vendor16);
    
    if (a == 1)
        goto gpu_pci_end; // Pekne jasne vystizne. Poslat ostatnych do riti
    
    if (a == 2) {
        uefi_println(SystemTable->ConOut, L"Failed to recognise CPU, not registered in database.");
    }

    
    uefi_println(SystemTable, L"----- GPU (PCI/e) -----");
    
    EFI_HANDLE *Handles = NULL;
    UINTN Count = 0;

    EFI_STATUS st = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiPciIoProtocolGuid,
        NULL,
        &Count,
        &Handles
    );

    if (EFI_ERROR(st)) {
        uefi_println(SystemTable, L"PCI scan failed.");
        return EFI_SUCCESS;
    }

    CHAR16 buf_count[25];
    decimalConvert(Count, buf_count, 25);

    uefi_print(SystemTable, L"Detected devices (PCI/e): ");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, buf_count);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");

    for (UINTN i = 0; i < Count; i++) {
        EFI_PCI_IO_PROTOCOL *PciIo;
        EFI_STATUS st = gBS->HandleProtocol(
            Handles[i],
            &gEfiPciIoProtocolGuid,
            (void**)&PciIo
        );

        EFI_STATUS ast = PciIo->Attributes(
            PciIo,
            EfiPciIoAttributeOperationEnable,
            EfiPciIoAttributeBusMaster |
            EfiPciIoAttributeMemory |
            EfiPciIoAttributeIo,
            NULL
        );

        if (EFI_ERROR(ast)) continue;
        if (EFI_ERROR(st)) continue;

        UINT16 VendorId;
        EFI_STATUS pst = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, 0x00, 1, &VendorId);
        
        if (EFI_ERROR(pst)) continue;
        if (VendorId == 0x0000 || VendorId == 0xFFFF) continue;

        UINT32 Class;
        if (EFI_ERROR(PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, 0x08, 1, &Class))) {
            uefi_println(SystemTable, L"PCI read failed");
            continue;
        }
        
        UINT8 BaseClass = (Class >> 24) & 0xFF;
        //UINT8 SubClass  = (Class >> 16) & 0xFF;
        //UINT8 ProgIf    = (Class >> 8)  & 0xFF;

        if (BaseClass != 0x03 && VendorId != 0x00) {
            continue;
        }

        CHAR16 vendor_name[32];
        int i = 0; // If this is 1 the vendorId is not regognised
        ascii_to_char16(pci_vendor_name(VendorId, &i), vendor_name, 32); // add if unknow

        uefi_print(SystemTable, L"Found device: ");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, vendor_name);
        if (i != 0) {
            uefi_print(SystemTable, L" 0x");
            CHAR16 vendorid[25];
            decimalConvert(VendorId, vendorid, 25);
            uefi_print(SystemTable, vendorid);
        }
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");
    }
    gBS->FreePool(Handles);
gpu_pci_end:

    uefi_println(SystemTable, L" --- GPU --- ");
    EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_HANDLE *GopHandles;
    UINTN GopCount;

    EFI_STATUS Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        &GopCount,
        &GopHandles
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Failed GPU lookup... (Terminating)");
        goto gpu_gop_end;
    }
    uefi_print(SystemTable, L"[*] GPU (GOP) devices found: ");
    CHAR16 buf[8];
    decimalConvert(GopCount, buf, 8);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
    uefi_println(SystemTable, L"");
    uefi_println(SystemTable, L"[+] Identifing GPU's...");

    for (UINTN i = 0; i < GopCount; i++) {
        EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
        EFI_STATUS Status = gBS->HandleProtocol(
            GopHandles[i],
            &gEfiGraphicsOutputProtocolGuid,
            (void**)&Gop
        );
        if (EFI_ERROR(Status)) continue;

        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info = Gop->Mode->Info;
        CHAR16 buf[16];

        uefi_print(SystemTable, L"\r\nGPU ");
        CHAR16 gpu_count[32];
        decimalConvert(i, gpu_count, 32);
        uefi_print(SystemTable, gpu_count);
        uefi_println(SystemTable, L"");
        uefi_print(SystemTable, L"GPU resolution: ");
        decimalConvert(Info->HorizontalResolution, buf, 16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
        uefi_print(SystemTable, L"x");
        decimalConvert(Info->VerticalResolution, buf, 16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
        uefi_print(SystemTable, L", Pixels per scanline: ");
        decimalConvert(Info->PixelsPerScanLine, buf, 16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
        uefi_print(SystemTable, L"\r\n");

        uefi_print(SystemTable, L"Framebuffer base: 0x");
        decimalConvert((UINTN)Gop->Mode->FrameBufferBase, buf, 16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);

        uefi_print(SystemTable, L", Size: ");
        decimalConvert(Gop->Mode->FrameBufferSize, buf, 16);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
        uefi_println(SystemTable, L"\r\n");
    }

gpu_gop_end:
    uefi_println(SystemTable, L"--- RAM ---");
    uefi_println(SystemTable, L"[+] Initializing Virt to Phys passthru\r\n");

    EFI_PHYSICAL_ADDRESS pml4_addr;
    EFI_PHYSICAL_ADDRESS pdpt_addr;
    EFI_PHYSICAL_ADDRESS pd_addr[4];

    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        1,
        &pml4_addr
    );

    if (EFI_ERROR(Status))
        goto panic;

    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        1,
        &pdpt_addr
    );

    if (EFI_ERROR(Status))
        goto panic;

    for (int i = 0; i < 4; i++) {
        Status = gBS->AllocatePages(
            AllocateAnyPages,
            EfiLoaderData,
            1,
            &pd_addr[i]
        );

        if (EFI_ERROR(Status))
            goto panic;
    }

    gBS->SetMem((VOID *)(UINTN)pml4_addr, 0, PAGE_SIZE);
    gBS->SetMem((VOID *)(UINTN)pdpt_addr, 0, PAGE_SIZE);

    for (int i = 0; i < 4; i++)
        gBS->SetMem((VOID *)(UINTN)pd_addr[i], 0, PAGE_SIZE);

    uint64_t *pml4 = (uint64_t *)(UINTN)pml4_addr;
    uint64_t *pdpt = (uint64_t *)(UINTN)pdpt_addr;

    pml4[0] =
        (uint64_t)pdpt_addr |
        PTE_PRESENT |
        PTE_RW;

    pml4[256] =
        (uint64_t)pdpt_addr |
        PTE_PRESENT |
        PTE_RW;

    for (int j = 0; j < 4; j++) {

        pdpt[j] =
            (uint64_t)pd_addr[j] |
            PTE_PRESENT |
            PTE_RW;

        uint64_t *pd =
            (uint64_t *)(UINTN)pd_addr[j];

        for (uint64_t i = 0; i < 512; i++) {

            uint64_t physical =
                ((uint64_t)j * 0x40000000ULL) +
                (i * HUGE_PAGE);

            pd[i] =
                physical |
                PTE_PRESENT |
                PTE_RW |
                PTE_PS;
        }
    }
    uefi_print(SystemTable, L"Setup CR3: ");
    uefi_print_hex(SystemTable, (UINT64)pml4_addr);

    /*
    uefi_println(SystemTable, L"--- KERNEL ---");
    uefi_println(SystemTable, L"[+] Loading main kernel...");

    //goto kernel_load_end; // DISABLED, disabled load kernel for now.

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_FILE_PROTOCOL *root;
    EFI_FILE_PROTOCOL *kernelFile;
    EFI_FILE_PROTOCOL *fileProt;
    EFI_FILE_INFO *fileInfo = NULL;

    // Locate the file system protocol
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    Status = gBS->LocateProtocol(&fsGuid, NULL, (void**)&fs);
    if (EFI_ERROR(Status)) return Status;

    // Open the root volume
    Status = fs->OpenVolume(fs, &root);
    if (EFI_ERROR(Status)) return Status;
    Status = root->Open(
        root,
        &kernelFile,
        L"kernel.bin",
        EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) return Status;

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Failed to read kernel.");
        goto panic;
    }
    fileProt = kernelFile;
    EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;
    UINTN Size = 0;
    UINTN bufferSize = 0;
    Status = fileProt->GetInfo(fileProt, &gEfiFileInfoGuid, &bufferSize, NULL);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        uefi_println(SystemTable, L"[!] Failed to get file...");
        goto panic;
    }

    Status = gBS->AllocatePool(
        EfiLoaderData,
        bufferSize,
        (void**)&fileInfo
    );
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] AllocatePool failed");
        goto panic;
    }

    Status = fileProt->GetInfo(
        fileProt,
        &gEfiFileInfoGuid,
        &bufferSize,
        fileInfo
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] GetInfo failed");
        goto panic;
    }

    UINTN kernelSize = fileInfo->FileSize;
    uefi_println(SystemTable, L"[+] Reading file...");
    void *kernelBuffer = NULL;
    Status = gBS->AllocatePool(
        EfiLoaderData,
        kernelSize,
        &kernelBuffer
    );
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Failed to allocate kernel buffer");
        goto panic;
    }

    UINTN bytesRead = kernelSize;
    Status = fileProt->Read(
        fileProt,
        &bytesRead,
        kernelBuffer
    );

    if (EFI_ERROR(Status) || bytesRead != kernelSize) {
        uefi_println(SystemTable, L"[!] Failed to read kernel file");
        goto panic;
    }

    EFI_PHYSICAL_ADDRESS kernelAddr = 0x100000;
    Status = gBS->AllocatePages(
        AllocateAddress,
        EfiLoaderData,
        EFI_SIZE_TO_PAGES(kernelSize),
        &kernelAddr
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Failed to allocate pages for kernel.");
        goto panic;
    }

    CopyMem((void*)kernelAddr, kernelBuffer, kernelSize); // Copy kernel into memory

    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    /* First call: get required size *//*
    Status = gBS->GetMemoryMap(
        &MemoryMapSize,
        NULL,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );

    /* Allocate buffer (+ some slack) *//*
    MemoryMapSize += 2 * DescriptorSize;
    gBS->AllocatePool(
        EfiLoaderData,
        MemoryMapSize,
        (void**)&MemoryMap
    );

    /* Second call: get actual map *//*
    Status = gBS->GetMemoryMap(
        &MemoryMapSize,
        MemoryMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );

    */

    uefi_println(SystemTable, L"--- KERNEL ---");
    uefi_println(SystemTable, L"[+] Loading main kernel...");

    // goto kernel_end_end; // DISABLED, kernel always crashes

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_FILE_PROTOCOL *root;
    EFI_FILE_PROTOCOL *kernelFile;
    EFI_FILE_INFO *fileInfo = NULL;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_PROTOCOL *fileProt;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

    EFI_GUID LoadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    Status = gBS->HandleProtocol(
        ImageHandle,
        &LoadedImageGuid,
        (VOID**)&LoadedImage
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Failed to get loaded image");
        goto panic;
    }

    
    Status = gBS->HandleProtocol(
        LoadedImage->DeviceHandle,
        &fsGuid,
        (VOID**)&fs
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] No filesystem on device");
        goto panic;
    }
    
    /*

    EFI_HANDLE *handles;
    UINTN count;

    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &fsGuid,
        NULL,
        &count,
        &handles
    );

    if (EFI_ERROR(Status))
    {
        uefi_println(SystemTable, L"[F] No filesystem handles");
    }
    else
    {
        uefi_println(SystemTable, L"[+] Filesystem exists!");
    }
    */

    Status = fs->OpenVolume(
        fs,
        &root
    );
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] OpenVolume failed");
        goto panic;
    }

    Status = root->Open(
        root,
        &kernelFile,
        L"kernel.elf",
        EFI_FILE_MODE_READ,
        0
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] kernel.bin not found");
        goto panic;
    }

    
    EFI_GUID FileInfoGuid = EFI_FILE_INFO_ID;
    UINTN fileInfoSize = 0;
    Status = kernelFile->GetInfo(
        kernelFile,
        &FileInfoGuid,
        &fileInfoSize,
        NULL
    );

    if (Status != EFI_BUFFER_TOO_SMALL) {
        uefi_println(SystemTable, L"[!] GetInfo size failed");
        uefi_status_print(SystemTable, Status); // Does print EFI_BUFFER_TOO_SMALL???
        goto panic;
    }

    Status = gBS->AllocatePool(
        EfiLoaderData,
        fileInfoSize,
        (VOID**)&fileInfo
    );
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] FileInfo allocation failed");
        goto panic;
    }

    Status = kernelFile->GetInfo(
        kernelFile,
        &FileInfoGuid,
        &fileInfoSize,
        fileInfo
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] GetInfo failed");
        goto panic;
    }


    UINTN kernelSize = fileInfo->FileSize;
    uefi_println(SystemTable, L"[+] Kernel size found");
    VOID *kernelBuffer = NULL;

    Status = gBS->AllocatePool(
        EfiLoaderData,
        kernelSize,
        &kernelBuffer
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Kernel buffer failed");
        goto panic;
    }
    UINTN readSize = kernelSize;

    Status = kernelFile->Read(
        kernelFile,
        &readSize,
        kernelBuffer
    );

    if (EFI_ERROR(Status) || readSize != kernelSize)
    {
        uefi_println(SystemTable, L"[!] Kernel read failed");
        goto panic;
    }

    
    uefi_println(SystemTable, L"[+] Kernel loaded into buffer");
    EFI_PHYSICAL_ADDRESS kernelAddr = 0x100000;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        EFI_SIZE_TO_PAGES(kernelSize),
        &kernelAddr
    );
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Kernel pages failed");
        goto panic;
    }
    gBS->CopyMem(
        (VOID*)kernelAddr,
        kernelBuffer,
        kernelSize
    );
    
    uefi_println(SystemTable, L"[+] Kernel copied");

    uefi_print(SystemTable, L"kernelPhysAddr = ");
    uefi_print_hex(SystemTable, kernelAddr);

    uefi_print(SystemTable, L"kernelSize = ");
    uefi_print_hex(SystemTable, kernelSize);

    uefi_print(SystemTable, L"First 8 bytes = ");
    uefi_print_hex(SystemTable, *(UINT64 *)kernelAddr);

    Elf64_Ehdr *hdr = get_elf_header(kernelAddr);
    Elf64_Phdr *phdr = get_program_headers(kernelAddr);

    if (hdr->e_ident[0] == 0x7F &&
        hdr->e_ident[1] == 'E' &&
        hdr->e_ident[2] == 'L' &&
        hdr->e_ident[3] == 'F') {
        uefi_println(SystemTable, L"[+] Parsing kernel.elf");
    } else {
        uefi_println(SystemTable, L"[-] kernel.elf is not ELF");
        goto panic;
    }

    /*

    for (UINT16 i = 0; i < hdr->e_phnum; i++)
    {
        Elf64_Phdr *p = &phdr[i];

        if (p->p_type != 0x00000001)
            continue;

        EFI_PHYSICAL_ADDRESS addr = p->p_paddr;

        Status = gBS->AllocatePages(
            AllocateAddress,
            EfiLoaderData,
            EFI_SIZE_TO_PAGES(p->p_memsz),
            &addr
        );

        if (EFI_ERROR(Status))
            goto panic;

        gBS->CopyMem(
            (VOID *)p->p_vaddr,
            (UINT8 *)kernelAddr + p->p_offset,
            p->p_filesz
        );

        if (p->p_memsz > p->p_filesz)
        {
            gBS->SetMem(
                (VOID*)(addr + p->p_filesz),
                p->p_memsz - p->p_filesz,
                0
            );
        }

        uefi_print_hex(SystemTable, p->p_vaddr);
        uefi_print_hex(SystemTable, p->p_paddr);
        uefi_print_hex(SystemTable, p->p_filesz);
        uefi_print_hex(SystemTable, p->p_memsz);
        uefi_println(SystemTable, L"");
    }*/

    uint64_t kernel_phys_start = UINT64_MAX;
    uint64_t kernel_phys_end = 0;
    uint64_t kernel_virt_start = UINT64_MAX;
    uint64_t kernel_virt_end = 0;

    for (UINT16 i = 0; i < hdr->e_phnum; i++)
    {
        Elf64_Phdr *p = &phdr[i];

        if (p->p_type != 1)
            continue;

        EFI_PHYSICAL_ADDRESS addr = p->p_paddr;

        Status = gBS->AllocatePages(
            AllocateAddress,
            EfiLoaderData,
            EFI_SIZE_TO_PAGES(p->p_memsz),
            &addr
        );

        if (EFI_ERROR(Status))
            goto panic;

        gBS->CopyMem(
            (VOID *)p->p_vaddr,
            (UINT8 *)kernelAddr + p->p_offset,
            p->p_filesz
        );

        if (p->p_memsz > p->p_filesz) {
            gBS->SetMem(
                (VOID *)(p->p_vaddr + p->p_filesz),
                p->p_memsz - p->p_filesz,
                0
            );
        }

        if (p->p_paddr < kernel_phys_start)
            kernel_phys_start = p->p_paddr;

        if (p->p_paddr + p->p_memsz > kernel_phys_end)
            kernel_phys_end = p->p_paddr + p->p_memsz;

        if (p->p_vaddr < kernel_virt_start)
            kernel_virt_start = p->p_vaddr;

        if (p->p_vaddr + p->p_memsz > kernel_virt_end)
            kernel_virt_end = p->p_vaddr + p->p_memsz;
    }

    uefi_print(SystemTable, L"Entry = ");
    uefi_print_hex(SystemTable, hdr->e_entry);
    //goto end;
    

kernel_load_end:
    uefi_println(SystemTable, L" --- GPU GOP --- ");
    uefi_println(SystemTable, L"[+] Loading GOP mode.");
    EFI_GUID gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_HANDLE Handle;
    EFI_EVENT gopInstallEvent;
    VOID *GopRegistration = NULL;
    EFI_EVENT GopEvent;

    Status = gBS->LocateProtocol(
        &gopGUID,
        NULL,
        (VOID **)&gop
    );
    if (EFI_ERROR(Status)) {
        Status = gBS->CreateEvent(
            EVT_NOTIFY_SIGNAL,
            TPL_CALLBACK,
            GopInstalledCallback,
            NULL,
            &GopEvent
        );

        Status = gBS->RegisterProtocolNotify(
            &gopGUID,
            gopInstallEvent,
            GopRegistration
        );
        UINTN Index;
        gBS->WaitForEvent(1, &gopEventCallProceed, &Index);
        CHAR16 *IndexBuf[64];
        decimalConvert(Index, IndexBuf, 64);
        uefi_print(SystemTable, L"[+] Load GOP exited with Index: ");
        uefi_println(SystemTable, IndexBuf);
    }

    uefi_println(SystemTable, L"[*] Loaded GOP protocol.");
    // I guess now we can use GOP
    EFI_GUID gEfiEdidDiscoveredProtocolGuid = EFI_EDID_DISCOVERED_PROTOCOL_GUID;
    EFI_EDID_DISCOVERED_PROTOCOL *Edid;
    int pickhighestFlag = 0;
    int skip_monitor = 0;
    Status = gBS->LocateProtocol(
        &gEfiEdidDiscoveredProtocolGuid, 
        NULL, 
        (VOID **)&Edid
    );

    int preferClassic = 1;

    // Parse the edid
    if (Edid->SizeOfEdid < 128 || Edid->Edid == NULL) { // Docs say that available edid is bigger then 128
        if (preferClassic != 1) {
            uefi_println(SystemTable, L"[!] No available information about monitor, picking highest resolution.");
        } else {
            uefi_println(SystemTable, L"[!] No available information about monitor, prefering 1920x1080 resolution");
        }
        pickhighestFlag = 1;
        skip_monitor = 1;
        goto gop_pick;
    }
    
    UINT8 *edid = Edid->Edid;
    UINT8 *DTD = edid + 54;

    UINT16 PixelClock = DTD[0] | (DTD[1] << 8);
    if (PixelClock == 0) {
        uefi_println(SystemTable, L"[!] No valid pixel clock.");
        goto panic;
    }
    
    UINT16 HActive = ((DTD[2] + ((DTD[4] & 0xF0) << 4)));
    UINT16 VActive = ((DTD[5] + ((DTD[7] & 0xF0) << 4)));

    UINT16 HTotal = HActive + (DTD[3] + ((DTD[4] & 0x0F) << 8));
    UINT16 VTotal = VActive + (DTD[6] + ((DTD[7] & 0x0F) << 8));

    float PixelClockHz = PixelClock * 10000;
    float RefreshHz = PixelClockHz / (HTotal * VTotal);

    int printGOP = 1; // Later it would be cool to read a config and print based of it
    UINTN SizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINT32 choosenMode = 0;

gop_pick:
    // Terrible, ugly -> Will do the job for now (later I will audit and rewrite this) 
    if (preferClassic == 1) {
        pickhighestFlag = 0;
    }
    uefi_println(SystemTable, L"Modes available: ");
    for (UINTN i = 0; i < gop->Mode->MaxMode; i++)
    {
        Info = gop->Mode->Info;
        Status = gop->QueryMode(
            gop,
            i,
            &SizeOfInfo,
            &Info
        );

        uefi_print(SystemTable, L"\t");
        CHAR16 buffff[256];
        decimalConvert(Info->HorizontalResolution, buffff, sizeof(buffff));
        uefi_print(SystemTable, buffff);
        uefi_print(SystemTable, L"x");
        decimalConvert(Info->VerticalResolution, buffff, sizeof(buffff));
        uefi_println(SystemTable, buffff);

        if (EFI_ERROR(Status))
            continue;


        if (printGOP)
            printGOPStats(SystemTable, *Info);


        // Exact requested resolution
        if (Info->HorizontalResolution == HActive &&
            Info->VerticalResolution == VActive)
        {
            choosenMode = i;
            break;
        }


        // Prefer 1920x1080
        if (preferClassic &&
            Info->HorizontalResolution == 1920 &&
            Info->VerticalResolution == 1080)
        {
            choosenMode = i;
            break;
        }
    }
    uefi_println(SystemTable, L"Mode was choosen..");
    if (skip_monitor) {
        goto gop_end;
    }
    // Only on real hardware this is possible (my qemu doesnt allow me to do so) (read or access EDID)
    uefi_println(SystemTable, L"--- Monitor ---");
    uefi_print(SystemTable, L"Refresh rate: ");
    CHAR16 RefreshRateBuf[64];
    decimalConvert(RefreshHz, RefreshRateBuf, 64);
    uefi_println(SystemTable, RefreshRateBuf);
    uefi_print(SystemTable, L"Resolution: ");
    CHAR16 HorizontalResolutionBuf[32];
    CHAR16 VerticalResolutionBuf[32];
    decimalConvert(HActive, HorizontalResolutionBuf, 32);
    decimalConvert(VActive, VerticalResolutionBuf, 32);
    uefi_print(SystemTable, HorizontalResolutionBuf);
    uefi_print(SystemTable, L"x");
    uefi_print(SystemTable, VerticalResolutionBuf);

    // Now we gotta switch to GOP mode that was choosen
gop_end:
    uefi_print(SystemTable, L"[+] Choose mode: ");
    CHAR16 bufff[256];
    decimalConvert(gop->Mode->Info->HorizontalResolution, bufff, sizeof(bufff));
    uefi_print(SystemTable, bufff);
    uefi_print(SystemTable, L"x");
    decimalConvert(gop->Mode->Info->VerticalResolution, bufff, sizeof(bufff));
    uefi_println(SystemTable, bufff);

    uefi_println(SystemTable, L"Press any key to enter kernel...");
    EFI_INPUT_KEY k;
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &k) != EFI_SUCCESS)
        ;
    uefi_println(SystemTable, L"[+] Entering GOP mode..");
    Status = gop->SetMode(gop, choosenMode);
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, "[!] Failed to set GOP mode.");
        goto panic; // Later try to initialize other functions and let kernel do its own thing
    }
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    
    UINT32 *fb = (UINT32 *)gop->Mode->FrameBufferBase;
    UINTN width  = gop->Mode->Info->HorizontalResolution;
    UINTN height = gop->Mode->Info->VerticalResolution;
    UINTN pitch  = gop->Mode->Info->PixelsPerScanLine;

    
    // TODO: Put back original background
    for (UINTN y = 0; y < height; y++) {
        for (UINTN x = 0; x < width; x++) {
            fb[y * pitch + x] = 0x00102030; // dark background
        }
    }
    
    // Print some art for fun into the new buffer
    
    DrawBMP(
        fb,
        pitch,
        (width - 50 * 4) / 2,
        (height - 50 * 4) / 2,
        logo_bmp,
        4
    );
    
    
    /*
    for (UINTN y = 0; y < height; y++) {
        for (UINTN x = 0; x < width; x++) {
            fb[y * pitch + x] = 0x000000; // dark background
        }
    }
    */
    

    // Here we need to allocate buffers and pass them to the kernel (if the kernel loaded)
    // which indeed didn't so we gotta just skip to ass
    //goto panic; // Because of disabled kernel loading

    BootInfo *bootInfo;

    Status = gBS->AllocatePool(
        EfiLoaderData,
        sizeof(BootInfo),
        (VOID**)&bootInfo
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Failed to allocate pool for bootinfo...");
        goto panic;
    }

    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;

    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    Status = gBS->GetMemoryMap(
        &MemoryMapSize,
        NULL,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );

    MemoryMapSize += DescriptorSize * 8;
    Status = gBS->AllocatePool(
        EfiLoaderData,
        MemoryMapSize,
        (VOID**)&MemoryMap
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Memory map alloc failed");
        goto panic;
    }

    Status = gBS->GetMemoryMap(
        &MemoryMapSize,
        MemoryMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );

    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] Memory map failed");
        goto panic;
    }

    bootInfo->magic = BOOTINFOMAGIC;

    bootInfo->framebuffer_base =
        gop->Mode->FrameBufferBase;

    bootInfo->framebuffer_size =
        gop->Mode->FrameBufferSize;

    bootInfo->width =
        gop->Mode->Info->HorizontalResolution;

    bootInfo->height =
        gop->Mode->Info->VerticalResolution;

    bootInfo->pixels_per_scanline =
        gop->Mode->Info->PixelsPerScanLine;

    /*

    bootInfo->kernel_virtual_address = phdr->p_paddr;
    bootInfo->kernel_physical_address = kernelAddr;
    bootInfo->kernel_physical_address_start = kernelAddr;
    bootInfo->kernel_physical_address_end = kernelAddr + kernelSize;
    
    bootInfo->kernel_size = kernelSize;
    */

    bootInfo->kernel_physical_address = kernelAddr;
    bootInfo->kernel_physical_address_start = kernel_phys_start;
    bootInfo->kernel_physical_address_end   = kernel_phys_end;

    bootInfo->kernel_virtual_address = kernel_virt_start;

    bootInfo->kernel_size =
        kernel_virt_end - kernel_virt_start;

    bootInfo->memory_map = (uint64_t)MemoryMap;
    bootInfo->memory_map_size = MemoryMapSize;
    bootInfo->memory_descriptor_size = DescriptorSize;

    serial_print("bootInfo ptr: ");
    serial_print_hex((uint64_t)bootInfo);

    serial_print("magic: ");
    serial_print_hex(bootInfo->magic);

    //goto end;

    Status = gBS->ExitBootServices(ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        uefi_println(SystemTable, L"[!] ExitBootServices failed");
        uefi_status_println(SystemTable, Status);
        int success = 0;
        while (!success) {
            MemoryMapSize = 0;

            gBS->GetMemoryMap(
                &MemoryMapSize,
                NULL,
                &MapKey,
                &DescriptorSize,
                &DescriptorVersion
            );

            // add some extra space
            MemoryMapSize += DescriptorSize * 8;

            gBS->AllocatePool(
                EfiLoaderData,
                MemoryMapSize,
                (VOID**)&MemoryMap
            );

            Status = gBS->ExitBootServices(ImageHandle, MapKey);
            if (!EFI_ERROR(Status)) {
                success = 1;
            }

            if (MemoryMapSize > 1024*1024*1024)
                goto panic;
        }
    }

    serial_print("BootInfo address: ");
    serial_print_hex((uint64_t)&bootInfo);

    serial_print("Magic: ");
    serial_print_hex(bootInfo->magic);

    write_cr3(pml4_addr);

    //uefi_print_hex(SystemTable, kernelAddr); // Causes the error I think aswell
    
    //KernelEntry entry = (KernelEntry)(kernelAddr + 0x42);
    //entry(&bootInfo);

    serial_print("Booting into kernel...\n");

    KernelEntry entry = (KernelEntry)hdr->e_entry;
    entry(bootInfo);

    for (UINTN y = 0; y < height; y++) {
        for (UINTN x = 0; x < width; x++) {
            fb[y * pitch + x] = 0x00404040;
        }
    }
    serial_print("\n[BOOT] Returned to boot");
    /*
    
    cozette_data font = load_glyphs();
    uint8_t* chara = font.glyphs + ('K' * font.size);

    // How can I write it into the middle of the screen 20x bigger?
    int scale = 20;

    int glyph_width = 8 * scale;
    int glyph_height = font.size * scale;

    int x = (width - glyph_width) / 2;
    int y = (height - glyph_height) / 2;

    draw_glyph(chara, font.size, x, y, pitch, fb);
    */

    while(1)
        asm volatile("hlt");
panic:
    uefi_println(SystemTable, L"(panic) Boot process failed, halting...");
end:
    uefi_println(SystemTable, L"Stopped. Press any key to shutdown.");
    EFI_INPUT_KEY key;
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) != EFI_SUCCESS);
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;

}
