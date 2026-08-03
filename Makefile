.PHONY: all kernel bootloader run clean

all: kernel bootloader

kernel:
	$(MAKE) -C kernel

bootloader: kernel
	$(MAKE) -C bootloader

run:
	$(MAKE) -C bootloader run

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C bootloader clean
	find . -type f \( -name "*.EFI" -o -name "*.bin" -o -name "*.elf" -o -name "*.o" \) -delete