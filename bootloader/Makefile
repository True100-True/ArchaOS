CC = x86_64-w64-mingw32-gcc

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -nostdlib \
	-Wall -Wextra -Wpedantic \
	-Wl,-subsystem,10 -Wl,--entry=efi_main

TARGET = BOOTX64.EFI
KERNEL = ../kernel/kernel.elf
SOURCE = uefi.c
BOOT_DISK = img.img
BOOT_DISK_SIZE = 260M

RAMDISK = ../test_disk/ramdisk.cpio
RAMDISK_folder = ../common/initramfs/

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

$(BOOT_DISK): $(TARGET) $(KERNEL) $(RAMDISK)
	rm -f $@

	truncate -s $(BOOT_DISK_SIZE) $@
	sgdisk $(BOOT_DISK) \
		--clear \
		--new=1:2048:0 \
		--typecode=1:ef00 \
		--change-name=1:EFI \
		$@

	mkfs.fat -F32 -n EFI --offset=2048 $@
	mmd -i img.img@@1M ::/EFI
	mmd -i img.img@@1M ::/EFI/BOOT
	mcopy -i $@@@1M -s $(TARGET) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $@@@1M $(KERNEL) ::/kernel.elf
	mcopy -i $@@@1M $(RAMDISK) ::/ramdiskfs.cpio

$(RAMDISK): $(RAMDISK_folder)
	mkdir -p "$(dir $(abspath $@))"
	cd "$(RAMDISK_folder)" && find . -print0 | cpio --null -o --format=newc > "$(abspath $@)"

$(RAMDISK_folder):
	mkdir -p "$(RAMDISK_folder)/binit" "$(RAMDISK_folder)/adam"
	cp ../drivers/ready "$(RAMDISK_folder)/binit/"


run: $(BOOT_DISK)
	qemu-system-x86_64 \
		-machine pc \
		-m 1024M \
		-cpu qemu64,+sse,+sse2 \
		-bios /usr/share/ovmf/x64/OVMF.4m.fd \
		-drive file=$(BOOT_DISK),format=raw \
		-net none \
		-serial stdio \
		-device virtio-gpu \
		-display gtk 
debug: $(BOOT_DISK)
		qemu-system-x86_64 \
		-machine q35 \
		-m 512M \
		-cpu qemu64 \
		-bios /usr/share/ovmf/x64/OVMF.4m.fd \
		-drive file=$(BOOT_DISK),format=raw \
		-net none \
		-serial stdio \
		-device virtio-gpu \
		-display gtk \
		-gdb tcp::9000 \
		-d int,cpu_reset -D qemu.log \
		-S

# -device VGA,edid=off \


clean:
	rm -f $(TARGET) $(BOOT_DISK)

.PHONY: all run clean