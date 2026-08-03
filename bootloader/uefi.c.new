#include "efi.h"
#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#else
void uefi_println(EFI_SYSTEM_TABLE *st, const CHAR16 *str) {
    st->ConOut->OutputString(st->ConOut, str);
    st->ConOut->OutputString(st->ConOut, L"\r\n");
}
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;
    uefi_println(SystemTable, L"[SYSTEM] Compiled without CPUID.h (use gcc or clang)");
    return EFI_SUCCESS;
}
#endif

void uefi_println(EFI_SYSTEM_TABLE *st, const CHAR16 *str) {
    st->ConOut->OutputString(st->ConOut, str);
    st->ConOut->OutputString(st->ConOut, L"\r\n");
}

void uefi_print(EFI_SYSTEM_TABLE *st, const CHAR16 *str) {
    st->ConOut->OutputString(st->ConOut, str);
}

int get_cpu_info(EFI_SYSTEM_TABLE *st, CHAR16 *vendor16[13], CHAR16 *brand16[64]) {
    uefi_println(st, L" --- CPU --- ");
    unsigned int ext;
    __get_cpuid(0x80000000, ext, NULL, NULL, NULL);

    if (ext < 0x80000004) {
        uefi_println(st, L"[!] CPUID is not available on this CPU.");
        return 1;
    } else {
        char vendor[13];
        get_cpu_vendor(vendor);
        vendor_to_char16(vendor, vendor16);

        uefi_print(st, L"CPU Vendor string: ");
        st->ConOut->OutputString(st->ConOut, vendor16);
        st->ConOut->OutputString(st->ConOut, L"\r\n");

        char brand[50];
        get_cpu_brand(brand);
        ascii_to_char16(brand, brand16, 64);

        uefi_print(st, L"CPU: ");
        st->ConOut->OutputString(st->ConOut, brand16);
        st->ConOut->OutputString(st->ConOut, L"\r\n");
        return 0;
    }
} 

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;
    uefi_println(SystemTable, L"[SYSTEM] Booted stage1...");
    uefi_println(SystemTable, L"[SYSTEM] Printing machine stats: ");
    // -- Frimware vendor ---
    uefi_println(SystemTable, L" --- Firmware --- ");
    uefi_print(SystemTable, L"Firmware vendor: ");
    uefi_println(SystemTable, SystemTable->FirmwareVendor);

    // --- CPU ---
    CHAR16 vendor16[13];
    CHAR16 brand16[64];
    int return_code_gcpuinf = get_cpu_info(SystemTable, &vendor16, &brand16);
    
    uefi_println(SystemTable, L"Press any key to shutdown.");
    EFI_INPUT_KEY key;
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) != EFI_SUCCESS);
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    return EFI_SUCCESS;
}