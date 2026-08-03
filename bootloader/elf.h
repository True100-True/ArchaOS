#ifndef ELF_H
#define ELF_H

#include "efi.h"

typedef struct {
    unsigned char e_ident[16];
    UINT16      e_type;
    UINT16      e_machine;
    UINT32      e_version;
    UINT64      e_entry;
    UINT64      e_phoff;
    UINT64      e_shoff;
    UINT32      e_flags;
    UINT16      e_ehsize;
    UINT16      e_phentsize;
    UINT16      e_phnum;
    UINT16      e_shentsize;
    UINT16      e_shnum;
    UINT16      e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    UINT32 p_type;
    UINT32 p_flags;
    UINT64 p_offset;
    UINT64 p_vaddr;
    UINT64 p_paddr;
    UINT64 p_filesz;
    UINT64 p_memsz;
    UINT64 p_align;
} Elf64_Phdr;



Elf64_Ehdr *get_elf_header(EFI_PHYSICAL_ADDRESS kernel) {
    return (Elf64_Ehdr *)kernel;
}

Elf64_Phdr *get_program_headers(EFI_PHYSICAL_ADDRESS kernel) {
    Elf64_Ehdr *header = get_elf_header(kernel);
    return (Elf64_Phdr *)
        ((UINT8 *)kernel + header->e_phoff);
}


#endif //ELF_H