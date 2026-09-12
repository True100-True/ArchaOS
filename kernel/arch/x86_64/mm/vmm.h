#ifndef VMM_H
#define VMM_H

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"
#include "pmm.h"
#include "../io.h"

#include "../../../../common/fromboot.h"


//#define PAGE_SIZE  0x1000ULL // Already defined in PMM
#define PAGE_MASK  (~(PAGE_SIZE - 1))

#define PTE_PRESENT  (1ULL << 0)
#define PTE_RW       (1ULL << 1)
#define PTE_USER     (1ULL << 2)
// TODO: expand and implement vprotect();

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

#define PASSTHRU_BASE      0xFFFFFFFF80000000ULL
#define USER_SPACE_BASE  0x0000000000000000ULL

#define STACK_SIZE 4 * PAGE_SIZE // Or 16K for short

extern bool virtual_mem_initialized;

void init_vmm(BootInfo *boot);

uint64_t valloc(bool zero);
void vfree(uint64_t virt_addr);

#endif // VMM_H