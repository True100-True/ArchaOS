#include "vmm.h"

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
void vfree(uint64_t virt_addr) {
    uint64_t pml4_phys_addr = read_cr3() & PTE_ADDR_MASK;
    //uint64_t *pml4 = get_virt_addr(pml4_phys_addr); // unmap_page has a conversion

    uint64_t phys_addr = unmap_page(virt_addr, pml4_phys_addr);
    if (phys_addr != 0)
        pfree(phys_addr); // we can extract from pml4 (offset) ?

    return;
}
/*
Init vmm maps kernel, framebuffer to new addresses
*/

void map_range(uint64_t phys, uint64_t virt, uint64_t size, uint64_t pml4_phys) {
    for (uint64_t off = 0; off < size; off += PAGE_SIZE)
        map_page(phys + off, virt + off, pml4_phys);
}

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

    uint64_t *pml4 = (uint64_t *)pml4_phys;
    uint64_t *pdpt = (uint64_t *)pdpt_phys;
    uint64_t *pd   = (uint64_t *)pd_phys;
    uint64_t *pt   = (uint64_t *)pt_phys;

    memset(pml4, 0, PAGE_SIZE);
    memset(pdpt, 0, PAGE_SIZE);
    memset(pd,   0, PAGE_SIZE);
    memset(pt,   0, PAGE_SIZE);

    print("[~] Table components:\n");

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

    print("\t[~] Framebuffer..");
    // Map framebuffer
    map_range(boot->framebuffer_base, boot->framebuffer_base, boot->framebuffer_size, pml4_phys);
    print("DONE\n");
    
    print("\t[~] Passthru..");
    // Map passthru
    map_range(0, PASSTHRU_BASE, max_physical_address, pml4_phys);
    print("DONE\n");

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
        boot->memory_map_size,
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

