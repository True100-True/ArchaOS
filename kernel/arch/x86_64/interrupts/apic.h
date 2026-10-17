#ifndef APIC_H
#define APIC_H

#include "../cpu/msr/msr.h"
#include "../../../lib/stdio.h"
#include "../timer/clock.h"
#include "idt.h"

// ACPI handlers
//extern void acpi_spurius_handler(void);

void apic_init();
void apic_start();

void apic_timer_start(void);
void apic_timer_stop(void);

void apic_eoi(void);

extern void spurious_int_handler(void);

#endif // APIC_H