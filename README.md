<div align="center">

### About

ArchaOS is in early development, targetting to be UNIX-like platform with inspiration from TempleOS and Linux.
Its ment to be available for anybody free of charge.
In this stage its useless, doesn't have proper userspace or any IO at all.

##### Notes

* You can see plain files without any content or unused files.
They are work in progress files and remiders for me to work on them.
* There never was 0.0.1 because we started from 0.1. 0.1.1 update is expansion to 0.1.
* Planing to be usable by update 0.2 

#### TODO path

**Bootloader**

- [X] Primitive UEFI Bootloader
- [X] Primitive startup

**Kernel**

- [X] Basic output
- [X] Basic memory handling
- [X] Memory handling
- [ ] interrupts
- [ ] threads
- [ ] scheduler
- [ ] kernel side syscalls
- [ ] i386 architecture
- [ ] RISC-V architecture
- [ ] Graphics

**Userspace (playground)**

- [ ] Playground
- [ ] syscall's
- [ ] Userland
- [ ] Driverland
- [ ] Networkland

**Other**
- [X] Logo
- [ ] Proper bootscreen
- [ ] Desktop

### Architecture

Architecture or ideology of kernel is simple. You have a kernel or supervisor that provides with basic resources.
These resources are fetched to playground. Playground is a mental model of this OS, every land such as userland,
driverland, networkland, etc. lives or executes on playground. This is ment to be much more robust for error handling, crash handling
and unexpected behavior as all of OS will stay safe and will not crach because of a single driver crash.

However as playground is a mental model the physical side is different. All of playground and multiple other services are
mostly executed by kernel. What does this mean? It means that all of playground (The main space) are executables executed by kernel.
This reduces the size of kernel. Aswell as it can be maintained directly booted from kernel (On ArchaOS developing ArchaOS).

### Run Guide

To run you have to compile it first. Go to root of the directory

```Shell
cd ~/ArchaOS/
```

Compile gcc and binutils for target: x86_64-elf

Install needed dependencies.

```Shell
sudo apt install build-essential bison flex libgmp3-dev libmpfr-dev libmpc-dev texinfo libisl-dev make
```

Install, configure and run binutils.

```Shell
cd ~/ArchaOS/kernel/toolchain
wget https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
tar xf binutils-2.45.tar.xz

mkdir build-binutils
cd build-binutils

../binutils-2.45/configure \
    --target=x86_64-elf \
    --prefix=$HOME/cross/bin \
    --with-sysroot \
    --disable-nls \
    --disable-werror
  
make -j$(nproc)
make install

x86_64-elf-as --version
x86_64-elf-ld --version
```

Install, configure and run gcc.

```Shell
cd ~/ArchaOS/kernel/toolchain
wget https://ftp.gnu.org/gnu/gcc/gcc-15.2.0/gcc-15.2.0.tar.xz
tar xf gcc-15.2.0.tar.xz

cd gcc-15.2.0
./contrib/download_prerequisites
cd ..

mkdir build-gcc
cd build-gcc

../gcc-15.2.0/configure \
    --target=x86_64-elf \
    --prefix=$HOME/cross/bin \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers
  
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)

make install-gcc
make install-target-libgcc

x86_64-elf-gcc --version
x86_64-elf-g++ --version
```

Add ~/cross/bin to your PATH

```Shell
export PATH=$PATH:~/cross/bin
```

Install mingw gcc (used for making .EFI files)

```Shell
sudo apt install  gcc-mingw-w64
```

Install qemu

```Shell
sudo apt install qemu-system
```

`
And make and run

```Shell
sudo make all run
```

process requires sudo permission to create .img that is bootable.

### Credit

**The Basekernel Operating System Kernel** at https://github.com/dthain/basekernel
    For amazing kernel, and readable kernel. Thanks so much.
**OSDev** at https://osdev.org
    For documentation about machine behavior
**uefi.org** at https://uefi.org
    For UEFI documentation.

### Software Requirements

For testing:

* qemu-system-x86_64

Build / compile:

* build-essential
* bison flex
* ibgmp3-dev
* libmpfr-dev
* libmpc-dev
* texinfo
* libisl-dev
* make

### Minimum Requirements

* **CPU:** Any 64-bit
* **RAM:** >512MB DDR1
* **GPU:** Any supporting UEFI GOP
* **STORAGE:** 16GB HDD
* **UEFI:** >v2.1
