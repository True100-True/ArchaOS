#include "../lib/stdio.h"
#include "pnc.h"
#include <stdint.h>
/*
Division Error 	0 (0x0) 	Fault 	#DE 	No
Debug 	1 (0x1) 	Fault/Trap 	#DB 	No
Non-maskable Interrupt 	2 (0x2) 	Interrupt 	- 	No
Breakpoint 	3 (0x3) 	Trap 	#BP 	No
Overflow 	4 (0x4) 	Trap 	#OF 	No
Bound Range Exceeded 	5 (0x5) 	Fault 	#BR 	No
Invalid Opcode 	6 (0x6) 	Fault 	#UD 	No
Device Not Available 	7 (0x7) 	Fault 	#NM 	No
Double Fault 	8 (0x8) 	Abort 	#DF 	Yes (Zero)
Coprocessor Segment Overrun 	9 (0x9) 	Fault 	- 	No
Invalid TSS 	10 (0xA) 	Fault 	#TS 	Yes
Segment Not Present 	11 (0xB) 	Fault 	#NP 	Yes
Stack-Segment Fault 	12 (0xC) 	Fault 	#SS 	Yes
General Protection Fault 	13 (0xD) 	Fault 	#GP 	Yes
Page Fault 	14 (0xE) 	Fault 	#PF 	Yes
Reserved 	15 (0xF) 	- 	- 	No
x87 Floating-Point Exception 	16 (0x10) 	Fault 	#MF 	No
Alignment Check 	17 (0x11) 	Fault 	#AC 	Yes
Machine Check 	18 (0x12) 	Abort 	#MC 	No
SIMD Floating-Point Exception 	19 (0x13) 	Fault 	#XM/#XF 	No
Virtualization Exception 	20 (0x14) 	Fault 	#VE 	No
Control Protection Exception 	21 (0x15) 	Fault 	#CP 	Yes
Reserved 	22-27 (0x16-0x1B) 	- 	- 	No
Hypervisor Injection Exception 	28 (0x1C) 	Fault 	#HV 	No
VMM Communication Exception 	29 (0x1D) 	Fault 	#VC 	Yes
Security Exception 	30 (0x1E) 	Fault 	#SX 	Yes
Reserved 	31 (0x1F) 	- 	- 	No 
*/
void divide_error()
{
    kernel_panic("DIVIDE BY ZERO");
}

void invalid_opcode() {
    kernel_panic("INVALID OPCODE");
}

void general_protection() {
    kernel_panic("GENERAL PROTECTION");
}

void page_fault() {
    kernel_panic("PAGE FAULT");
}