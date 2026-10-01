.PHONY: all kernel bootloader run debug clean

all: kernel bootloader

kernel:
	$(MAKE) -C kernel
bootloader: kernel
	$(MAKE) -C bootloader

run: bootloader
	$(MAKE) -C bootloader run
debug: bootloader
	$(MAKE) -C bootloader debug

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C bootloader clean
	find . -type f \( -name "*.EFI" -o -name "*.bin" -o -name "*.elf" -o -name "*.o" \) -delete
# Flash to usb sudo dd of=[USB] if=./bootloader/img.img bs=4M status=progress