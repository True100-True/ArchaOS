#ifndef INIT_X86_64_H
#define INIT_X86_64_H

#include "../../lib/stdio.h"

#include "io.h"

#include "cpu/gdt/gdt.h"
#include "cpu/tss/tss.h"
#include "interrupts/idt.h"

#include "mm/pmm.h"
#include "mm/vmm.h"

#include "../../errs/pnc.h"
#include "../../../common/fromboot.h"

void init_cpu();
void init_memory(BootInfo *boot);
void process();

#endif // INIT_X86_64_H