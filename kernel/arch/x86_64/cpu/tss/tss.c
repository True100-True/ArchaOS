#include "tss.h"

void init_tss() {
    struct tss_t tss;

    tss.rsp0 = 0;
    //tss.rsp1 = 0; // We do not set 
    //tss.rsp2 = 0; // bc these rings are useless tbh

    tss.ist1 = 0;
    tss.ist2 = 0;
    tss.ist3 = 0;
    tss.ist4 = 0;
    tss.ist5 = 0;
    tss.ist6 = 0;
    tss.ist7 = 0;

    tss.iomap_base = sizeof(tss);

    //load_tss(tss);
}