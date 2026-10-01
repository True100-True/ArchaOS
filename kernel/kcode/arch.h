#ifndef KCODE_ARCH_H
#define KCODE_ARCH_H

#include <stdint.h>
#include "../config.h"

#ifndef X86_64
#include "../arch/x86_64/init.h"
#include "../arch/x86_64/io.h"

#include "../arch/x86_64/mm/pmm.h"
#include "../arch/x86_64/mm/vmm.h"

#include "../arch/x86_64/thread/thread.h"
#endif



#endif