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

extern void divide_handler();
extern void invalid_opcode_handler();
extern void general_protection_handler();
extern void page_fault_handler();
extern void double_fault_handler();

void init_cpu() {
    bool status;
    print("Initializing ARCH (x86_64)\n\n");
    asm("cli");
    print("[+] Loading GDT..");
    init_gdt();
    print("OK\n"); // [~] Loaded GDT
    print("[+] Loading IDT..");
    // While loading IDT there are 3 "stages"
    // Creation of IDT
    // PIC / APIC
    // load

    //print("Creating entry..\n");

    struct idt_entry idt[256];

    // Faults (cpu raised)
    idt_set_gate(idt, 0, divide_handler);
    idt_set_gate(idt, 6, invalid_opcode_handler);
    idt_set_gate(idt, 13, general_protection_handler);
    idt_set_gate(idt, 14, page_fault_handler);
    idt_set_gate(idt, 8, double_fault_handler);

    //print("Allocated CPU faults..\n");

    //print("Initializing PIC\n");

    // Supports APIC?
    struct cpuid_result cres = cpuid_(1, 0); // leaf 1, subleaf 0

    bool support_apic   = (cres.edx & (1 << 9)) != 0;
    bool support_x2apic = (cres.ecx & (1 << 21)) != 0;

    if (support_x2apic) {
        //print("[+] x2APIC supported");
        //x2apic_enable(); // near future
        pic_disable_total();
    } else if (support_apic) {
        //print("[+] APIC supported");
        //apic_enable();
        pic_disable_total();
    } else {
        // This rarely happens on modern CPUs
        //print("[-] PIC supported");
        pic_init(0x20, 0x28); // Standard

        pic_enable(0);
    }



    /*
    print("enabling PIC 0\n");
    pic_enable(0);
    print("enable IDT\n");
    idt_set_gate(idt, 0x20, tick_interrupt); // irq 0 is 0x20 vector
    */

    // Load
    //print("Loading IDT\n");
    load_idt(sizeof(idt)-1, (uint64_t)&idt);
    
    print("OK\n");
    asm("sti");
    print("[+] Loading SYSCALL..");
    status = init_syscalls();
    if (status == true) {
        print("OK\n");
    } else {
        print("FAIL\n");
    }
    
    print("\nCPU -- Finished\n\n");
}
