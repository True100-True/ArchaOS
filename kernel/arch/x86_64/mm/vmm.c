#include "vmm.h"
#include "../../../errs/pnc.h"

/*
Important!!!
functions starting with p are imported from pmm.c
such as palloc() pfree() etc. are physical NOT virtual
palloc returns physical address !!! NOT accessible after init (accessible before since bootloader maps identity virtual to physical)
    to access it convert it to virt using get_virt_addr(physical addr returned by pallloc)

MEMORY MODEL:

*/

// helpers 

// Used to access physical address -- "bypassing" MMU (not really but we can access phys addresses)
uint64_t* get_virt_addr(uint64_t phys_addr) {
    return (uint64_t*)(phys_addr + PASSTHRU_BASE);
}

bool virtual_mem_initialized = false;

static inline uint64_t pml4_index(uint64_t v) {
    return (v >> 39) & 0x1FF;
}
static inline uint64_t pdpt_index(uint64_t v) {
    return (v >> 30) & 0x1FF;
}
static inline uint64_t pd_index(uint64_t v) {
    return (v >> 21) & 0x1FF;
}
static inline uint64_t pt_index(uint64_t v) {
    return (v >> 12) & 0x1FF;
}
static inline void invlpg(uint64_t addr) {
    asm volatile ("invlpg (%0)" :: "r"(addr) : "memory");
}
static inline uint64_t *page_table_ptr(uint64_t phys) {
    if (!virtual_mem_initialized)
        return (uint64_t *)phys;

    return get_virt_addr(phys);
}

void map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t pml4_phys) {
    uint64_t *pml4 = page_table_ptr(pml4_phys);

    uint64_t pml4_i = pml4_index(virt_addr);
    uint64_t pdpt_i = pdpt_index(virt_addr);
    uint64_t pd_i   = pd_index(virt_addr);
    uint64_t pt_i   = pt_index(virt_addr);


    if (!(pml4[pml4_i] & PTE_PRESENT)) {
        uint64_t pdpt_phys = palloc();
        if (!pdpt_phys)
            kernel_panic("Out of memory creating PDPT");
        uint64_t *pdpt = page_table_ptr(pdpt_phys);

        memset(pdpt, 0, 0x1000);

        pml4[pml4_i] =
            (pdpt_phys & PTE_ADDR_MASK) |
            PTE_PRESENT |
            PTE_RW;
    }

    uint64_t pdpt_phys = pml4[pml4_i] & PTE_ADDR_MASK;
    uint64_t *pdpt = page_table_ptr(pdpt_phys);

    if (!(pdpt[pdpt_i] & PTE_PRESENT)) {
        uint64_t pd_phys = palloc();
        if (!pd_phys)
            kernel_panic("Out of memory creating PD");
        uint64_t *pd = page_table_ptr(pd_phys);

        memset(pd, 0, 0x1000);

        pdpt[pdpt_i] =
            (pd_phys & PTE_ADDR_MASK) |
            PTE_PRESENT |
            PTE_RW;
    }

    uint64_t pd_phys = pdpt[pdpt_i] & PTE_ADDR_MASK;

    uint64_t *pd = page_table_ptr(pd_phys);

    if (!(pd[pd_i] & PTE_PRESENT)) {
        uint64_t pt_phys = palloc();
        if (!pt_phys)
            kernel_panic("Out of memory creating PT");
        uint64_t *pt = page_table_ptr(pt_phys);

        memset(pt, 0, 0x1000);

        pd[pd_i] =
            (pt_phys & PTE_ADDR_MASK) |
            PTE_PRESENT |
            PTE_RW;
    }

    uint64_t pt_phys = pd[pd_i] & PTE_ADDR_MASK;

    uint64_t *pt = page_table_ptr(pt_phys);

    pt[pt_i] =
        (phys_addr & PTE_ADDR_MASK) |
        PTE_PRESENT |
        PTE_RW;

    invlpg(virt_addr);
}
uint64_t unmap_page(uint64_t virt_addr, uint64_t pml4_phys) {
    uint64_t *pml4_table = page_table_ptr(pml4_phys);

    uint64_t pml4_i = pml4_index(virt_addr);
    uint64_t pdpt_i = pdpt_index(virt_addr);
    uint64_t pd_i   = pd_index(virt_addr);
    uint64_t pt_i   = pt_index(virt_addr);

    if (!(pml4_table[pml4_i] & PTE_PRESENT))
        return 0;
    uint64_t *pdpt_table = page_table_ptr((pml4_table[pml4_i] & PTE_ADDR_MASK));
    
    if (!(pdpt_table[pdpt_i] & PTE_PRESENT))
        return 0;
    uint64_t *pd_table = page_table_ptr((pdpt_table[pdpt_i] & PTE_ADDR_MASK));
    
    if (!(pd_table[pd_i] & PTE_PRESENT))
        return 0;
    uint64_t *pt_table = page_table_ptr((pd_table[pd_i] & PTE_ADDR_MASK));
    uint64_t phys_addr = pt_table[pt_i] & PTE_ADDR_MASK;

    pt_table[pt_i] = 0;
    invlpg(virt_addr);
    return phys_addr; 
}

/*
// THIS IS A PROOF OF CONCEPT -- rewrite and implement buddy rather then simple bump
static uint64_t next_virt = 0x1000;
uint64_t valloc(bool zero) {
    uint64_t pml4_phys_addr = read_cr3() & PTE_ADDR_MASK;
    //uint64_t *pml4 = get_virt_addr(pml4_phys_addr); // map_page has a conversion

    uint64_t phys_addr = palloc(); // Free / available phys page
    if (!phys_addr)
        return 0;
    
    uint64_t virt_addr = next_virt;
    next_virt += PAGE_SIZE; // Bump allocator -> use buddy algorithm
    map_page(phys_addr, virt_addr, pml4_phys_addr);
    if (zero)
        memset((void *)virt_addr, 0, PAGE_SIZE); // why PAGE_SIZE? because that's how much we are moving with the bump alloc

    return virt_addr;
}

uint64_t vallocs(bool zero, uint64_t count) {
    if (count == 0)
        return 0;
    uint64_t pml4_phys_addr = read_cr3() & PTE_ADDR_MASK;
    uint64_t start_virt = next_virt;

    for (uint64_t i = 0; i < count; i++) {
        uint64_t phys_addr = palloc();
        if (!phys_addr) {
            vfrees(start_virt, i);

            return 0;
        }

        uint64_t virt_addr = start_virt + i * PAGE_SIZE;

        map_page(phys_addr, virt_addr, pml4_phys_addr);

        if (zero) {
            memset((void *)virt_addr, 0, PAGE_SIZE);
        }
    }

    next_virt += count * PAGE_SIZE;
    return start_virt;
}


void vfree(uint64_t virt_addr) {
    uint64_t pml4_phys_addr = read_cr3() & PTE_ADDR_MASK;
    //uint64_t *pml4 = get_virt_addr(pml4_phys_addr); // unmap_page has a conversion

    uint64_t phys_addr = unmap_page(virt_addr, pml4_phys_addr);
    if (phys_addr != 0)
        pfree(phys_addr); // we can extract from pml4 (offset) ?

    return;
}

void vfrees(uint64_t virt_addr, uint64_t count) {
    uint64_t pml4_phys_addr = read_cr3() & PTE_ADDR_MASK;
    //uint64_t *pml4 = get_virt_addr(pml4_phys_addr); // unmap_page has a conversion

    for (uint64_t i = 0; i < count; i++) {
        uint64_t phys_addr = unmap_page(virt_addr, pml4_phys_addr);
        if (phys_addr != 0)
            pfree(phys_addr); // we can extract from pml4 (offset) ?
        virt_addr += PAGE_SIZE;
    }
    return;
}
    */

static uint64_t next_virt = KHEAP_BASE;

uint64_t valloc(bool zero)
{
    return vallocs(zero, 1);
}

uint64_t vallocs(bool zero, uint64_t count)
{
    if (count == 0)
        return 0;

    uint64_t pml4 =
        read_cr3() & PTE_ADDR_MASK;

    uint64_t start = next_virt;

    for (uint64_t i = 0; i < count; i++) {

        uint64_t phys = palloc();

        if (!phys) {
            vfrees(start, i);
            return 0;
        }

        uint64_t virt =
            start + i * PAGE_SIZE;

        map_page(phys, virt, pml4);

        if (zero)
            memset((void *)virt, 0, PAGE_SIZE);
    }

    next_virt += count * PAGE_SIZE;

    return start;
}

void vfree(uint64_t virt_addr)
{
    vfrees(virt_addr, 1);
}

void vfrees(uint64_t virt_addr, uint64_t count)
{
    if (count == 0)
        return;

    uint64_t pml4 =
        read_cr3() & PTE_ADDR_MASK;

    for (uint64_t i = 0; i < count; i++) {

        uint64_t virt =
            virt_addr + i * PAGE_SIZE;

        uint64_t phys =
            unmap_page(virt, pml4);

        if (phys)
            pfree(phys);
    }
}

/*
Init vmm maps kernel, framebuffer to new addresses
*/

void map_range(uint64_t phys, uint64_t virt, uint64_t size, uint64_t pml4_phys) {
    uint64_t phys_page = phys & PAGE_MASK;
    uint64_t virt_page = virt & PAGE_MASK;
    uint64_t offset = phys - phys_page;
    uint64_t mapped_size = size + offset;

    for (uint64_t off = 0; off < mapped_size; off += PAGE_SIZE)
        map_page(phys_page + off, virt_page + off, pml4_phys);
}

// Required to identify APIC 
// and map it as PTE_PWD | PTE_PCD (skip L3 cache)
typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory, // OUR data
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
   uint32_t    Type;
   uint64_t    PhysicalStart;
   uint64_t    VirtualStart;
   uint64_t    NumberOfPages;
   uint64_t    Attribute;
} EFI_MEMORY_DESCRIPTOR;

void init_vmm(BootInfo *boot) {
    print("[+] Starting init..\n");
    uint64_t *uefi_cr3 = (uint64_t*)read_cr3();
    print("UEFI CR3: %lx \n", uefi_cr3);

    // because bootloader idenity maps virtual to physical (1:1)
    // we can use pointers as physical and virtual memory 

    print("[+] Assembling pml4 table\n");

    uint64_t pml4_phys = palloc();
    uint64_t pdpt_phys = palloc();
    uint64_t pd_phys   = palloc();
    uint64_t pt_phys   = palloc();

    if (!pml4_phys || !pdpt_phys || !pd_phys || !pt_phys)
        kernel_panic("Out of memory creating VMM");

    uint64_t *pml4 = (uint64_t *)pml4_phys;
    uint64_t *pdpt = (uint64_t *)pdpt_phys;
    uint64_t *pd   = (uint64_t *)pd_phys;
    uint64_t *pt   = (uint64_t *)pt_phys;

    memset(pml4, 0, PAGE_SIZE);
    memset(pdpt, 0, PAGE_SIZE);
    memset(pd,   0, PAGE_SIZE);
    memset(pt,   0, PAGE_SIZE);

    print("[~] Table components:\n");

    // MemoryMap is in boot->memorymap
    // But how do we identify ACPI?

    pml4[0] =
        pdpt_phys |
        PTE_PRESENT |
        PTE_RW;

    pdpt[0] =
        pd_phys |
        PTE_PRESENT |
        PTE_RW;

    pd[0] =
        pt_phys |
        PTE_PRESENT |
        PTE_RW;

    for (uint64_t i = 0; i < 512; i++) {
        pt[i] =
            (i * PAGE_SIZE) |
            PTE_PRESENT |
            PTE_RW;
    }

    print("\tPML4 : %lx \n", (uint64_t)pml4);
    print("\tPDPT : %lx \n", (uint64_t)pdpt);
    print("\tPD   : %lx \n", (uint64_t)pd);

    /*
    Map kernel, kernel stack, framebuffer, etc.
    */

    print("[+] Mapping to table..\n");

    print("\t[~] Kernel..");
    // Map kernel 
    map_range(boot->kernel_physical_address_start, boot->kernel_virtual_address, boot->kernel_size, pml4_phys);
    print("DONE\n");

    // Map kernel stack
    print("\t[~] Stack..");
    uint64_t rsp = read_rsp();

    uint64_t stack_page = rsp & ~(PAGE_SIZE - 1);

    map_range(
        stack_page - 16 * PAGE_SIZE,
        stack_page - 16 * PAGE_SIZE,
        32 * PAGE_SIZE,
        pml4_phys
    );

    print("DONE\n");

    // RIP
    print("\t[~] Kernel RIP..");
    uint64_t current_rip = read_rip();
    uint64_t rip_page = current_rip & ~(PAGE_SIZE - 1);

    // Map the current RIP and a few pages around it 1:1 
    // so the CPU doesn't crash the moment CR3 changes.
    map_range(
        rip_page - PAGE_SIZE, 
        rip_page - PAGE_SIZE, 
        PAGE_SIZE * 10, 
        pml4_phys
    );
    print("DONE\n");

    print("\t[~] Framebuffer..");
    // Map framebuffer
    map_range(boot->framebuffer_base, boot->framebuffer_base, boot->framebuffer_size, pml4_phys);
    print("DONE\n");
    
    print("\t[~] Passthru..");
    // Map passthru
    map_range(0, PASSTHRU_BASE, max_physical_address, pml4_phys);
    print("DONE\n");

    print("\t[~] APIC..");
    // Loop thru Memory map and find APIC reserved mem chunks 
    uint64_t entries = boot->memory_map_size / boot->memory_descriptor_size;
    for(uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*) (
                (uint8_t*)boot->memory_map + i * boot->memory_descriptor_size );
        if (desc->Type == EfiConventionalMemory) {
            switch (desc->Type) {
                case EfiConventionalMemory:
                case EfiLoaderCode:
                case EfiLoaderData:
                case EfiBootServicesCode:
                case EfiBootServicesData:
                case EfiRuntimeServicesCode:
                case EfiRuntimeServicesData:
                case EfiACPIReclaimMemory:
                    map_range(desc->PhysicalStart, desc->PhysicalStart, desc->NumberOfPages * PAGE_SIZE, pml4_phys);
                    break;
                case EfiACPIMemoryNVS:
                    map_range(desc->PhysicalStart, desc->PhysicalStart, desc->NumberOfPages * PAGE_SIZE, pml4_phys);
                    break;
            }
        }
    }
    // Can not forget about APIC MMIO (~4kilobytes in size)
    uint64_t apic_record = rdmsr(0x1B);
    uint64_t apic_page = (apic_record >> 12) & 0xFFFFFULL;
    uint64_t apic_base = apic_page << 12;
    map_range(apic_base, apic_base, 0x4000, pml4_phys);

    print("OK\n");

    print("\t[~] Other..");
    // BootInfo
    uint64_t boot_addr = (uint64_t)boot;
    uint64_t boot_page = boot_addr & ~(PAGE_SIZE - 1);

    map_range(
        boot_page,
        boot_page,
        PAGE_SIZE,
        pml4_phys
    );

    // Memory map
    map_range(
        (uint64_t)boot->memory_map & ~(PAGE_SIZE - 1),
        (uint64_t)boot->memory_map & ~(PAGE_SIZE - 1),
        boot->memory_map_size + ((uint64_t)boot->memory_map & (PAGE_SIZE - 1)),
        pml4_phys
    );
    print("DONE\n");

    virtual_mem_initialized = true;

    print("init_vmm = %lx \n", (uint64_t)init_vmm);
    print("rip = %lx \n", read_rip());
    print("rsp = %lx \n", read_rsp());

    print("[~] Loading CR3..");

    asm volatile ("cli");

    write_cr3(pml4_phys);
    print("OK\n");

    // After this you can't access physical memory 1:1 as old cr3 will be not in effect
}
