#ifndef INIT_X86_64_H
#define INIT_X86_64_H

#include "../../lib/stdio.h"
#include "../../lib/stdbool.h"
#include "cpu/cpuid/cpuid.h"

#include "io.h"

#include "cpu/gdt/gdt.h"
#include "cpu/tss/tss.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/apic.h"
#include "syscalls/syscalls.h"
#include "timer/clock.h"
#include "timer/pit.h"

#include "mm/pmm.h"
#include "mm/vmm.h"

#include "../../errs/pnc.h"
#include "../../../common/fromboot.h"

void init_primitivs();
void init_cpu();
void init_memory(BootInfo *boot);
void process();



#endif // INIT_X86_64_H