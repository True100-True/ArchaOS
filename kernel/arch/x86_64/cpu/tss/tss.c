#include "tss.h"

struct tss_t tss;

void init_tss() {
    tss.rsp0 = 0; // kernel space, NEED stack pointer here
    //tss.rsp1 = 0; // We do not set 
    //tss.rsp2 = 0; // bc these rings are useless tbh

    // For now, change later
    tss.ist1 = 0;
    tss.ist2 = 0;
    tss.ist3 = 0;
    tss.ist4 = 0;
    tss.ist5 = 0;
    tss.ist6 = 0;
    tss.ist7 = 0;

    tss.iomap_base = sizeof(tss);
}

