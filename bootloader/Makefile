CC = x86_64-w64-mingw32-gcc

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -nostdlib \
	-Wall -Wextra -Wpedantic \
	-Wl,-subsystem,10 -Wl,--entry=efi_main

TARGET = BOOTX64.EFI
KERNEL = ../kernel/kernel.elf
SOURCE = uefi.c
DISK = img.img
DISK_SIZE = 260M

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

$(DISK): $(TARGET) $(KERNEL)
	rm -f $@

	# Create empty disk
	truncate -s $(DISK_SIZE) $@

	# Create GPT and one EFI System Partition
	sgdisk \
		--clear \
		--new=1:2048:0 \
		--typecode=1:ef00 \
		--change-name=1:EFI \
		$@

	# Format partition 1 as FAT32
	mkfs.fat -F32 -n EFI --offset=2048 $@
	
	mmd -i img.img@@1M ::/EFI
	mmd -i img.img@@1M ::/EFI/BOOT

	# Copy files into the FAT filesystem without mounting
	mcopy -i $@@@1M -s $(TARGET) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $@@@1M $(KERNEL) ::/kernel.elf


run: $(DISK)
	qemu-system-x86_64 \
		-machine q35 \
		-m 512M \
		-cpu qemu64 \
		-bios /usr/share/ovmf/x64/OVMF.4m.fd \
		-drive file=$(DISK),format=raw \
		-net none \
		-serial stdio \
		-device virtio-gpu \
		-display gtk 
debug: $(DISK)
		qemu-system-x86_64 \
		-machine q35 \
		-m 512M \
		-cpu qemu64 \
		-bios /usr/share/ovmf/OVMF.fd \
		-drive file=$(DISK),format=raw \
		-net none \
		-serial stdio \
		-device virtio-gpu \
		-display gtk \
		-gdb tcp::9000 \
		-d int,cpu_reset -D qemu.log \
		-S

# -device VGA,edid=off \


clean:
	rm -f $(TARGET) $(DISK)

.PHONY: all run clean