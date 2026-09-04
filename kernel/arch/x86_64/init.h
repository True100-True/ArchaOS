#ifndef INIT_X86_64_H
#define INIT_X86_64_H

#include "../../lib/stdio.h"

#include "gdt/gdt.h"
#include "idt/idt.h"

#include "mm/pmm.h"
#include "mm/vmm.h"

#include "../../errs/pnc.h"
#include "../../../common/fromboot.h"

void init_cpu();
void init_memory();

#endif // INIT_X86_64_H