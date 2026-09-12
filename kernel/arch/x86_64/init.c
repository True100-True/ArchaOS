#include "init.h"

// Because it is not as depended on architecture I would say we should keep part here part there
// lets just initialize the stuff such as cr3, mapping etc.

void test_ppm_func(uint64_t mem_map, uint64_t mem_map_size, uint64_t descriptor_size) {
    print("[TEST] Testing PMM\n");

    uint64_t page = palloc();
    if (page) {
        kernel_panic("Allocated before PMM init");
    }
    print("[+] Allocation blocked before init\n");
    for (uint64_t i = 0; i < 15; i++) {
        pfree((i * 0x1000));
    }

    print("Free pages before init: ");
    print_hex_(get_free_pages());

    print("\n[+] Initializing PMM..\n");
    init_pmm(mem_map, mem_map_size, descriptor_size);
    print("[~] Initialized PMM..\n");

    uint64_t before = get_free_pages();
    page = palloc();

    if (!page) {
        kernel_panic("Allocation failed");
    }
    print("Allocated page: ");
    print_hex_((uint64_t)page);
    print("\nFree pages: ");
    print_hex_(get_free_pages());

    pfree(page);
    print("\nAfter free: ");

    print_hex_(get_free_pages());
    if (get_free_pages() != before) {
        kernel_panic("Page count mismatch");
    }

    print("\n[+] PMM test passed\n");
}
void init_memory(BootInfo *boot) {
    print("[+] Loading PMM..\n");
    // init_pmm(boot->memory_map, boot->memory_map_size, boot->memory_descriptor_size);
    test_ppm_func(boot->memory_map, boot->memory_map_size, boot->memory_descriptor_size);
    print("[~] Loaded PMM..\n");
    print("[+] Loading VMM..\n");
    init_vmm(boot);
    print("[~] Loaded VMM..\n");
    print("\nMemory -- Finished\n");
}

void init_cpu() {
    print("Initializing ARCH (x86_64)\n\n");
    print("[+] Loading GDT..");
    init_gdt();
    print("OK\n"); // [~] Loaded GDT
    print("[+] Loading IDT..");
    init_idt();
    print("OK\n");
    print("\nCPU -- Finished\n\n");
}