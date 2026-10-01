#ifndef CONFIG_H
#define CONFIG_H

// About.h - basic config system
/*
My idea behind this,

1. 
You configure your kernel before your compilation starts 
or you will use existing configuration to match your system.

2.
If you can not configure your kernel (you don't know what ur computer is )
you let installer choose the right configuration for you
Installer will test, read and create model that will match your computer the best
however this will take a long time and will be implemented later.

3.
Pros
You won't have any unused bloats, drivers, etc. as your kernel wasnt even compiled to do so

Cons
If computer changes there may be some issues and you will need to recompile your kernel

//// THIS IDEA REQUIRES DECISIONS ////
- decided to keep it for Computer-to-Computer compatibility

*/

#define x86_64 1
#define QEMU_TEST 1 // WATCH OUT FOR THIS VALUE

#endif
