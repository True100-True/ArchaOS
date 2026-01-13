CC = x86_64-w64-mingw32-gcc
CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -nostdlib \
		-Wall -Wextra -Wpedantic \
		-Wl,-subsystem,10 -Wl,--entry=efi_main
TARGET = BOOTX64.EFI
SOURCE = uefi.c
DISK = img.img

all: $(TARGET) $(DISK)

run: $(TARGET) $(DISK)
	sudo qemu-system-x86_64 \
		 -m 512M \
		 -cpu qemu64,vendor="AuthenticAMD" \
		 -bios /usr/share/ovmf/OVMF.fd \
		 -drive file=$(DISK),format=raw \
		 -net none  \
		 -monitor stdio 
# 		 -device virtio-gpu-pci \
#  	     -display gtk
$(TARGET): $(SOURCE)
	rm -rf $(DISK)
	$(CC) $(CFLAGS) -o $@ $<

$(DISK): $(TARGET)
	cp $(TARGET) ./test/EFI/BOOT/
	dd if=/dev/zero of=$(DISK) bs=1M count=64

	parted $(DISK) --script \
		mklabel gpt \
		mkpart ESP fat32 1MiB 63MiB \
		set 1 esp on

	# Attach loop device with partitions
	sudo losetup -Pf $(DISK)

	# Find loop device name
	LOOP=$$(losetup -j $(DISK) | cut -d: -f1); \
	echo "Using $$LOOP"; \
	sudo mkfs.fat -F32 -n ESP $$LOOP"p1"; \
	sudo mkdir -p /tmp/efimount; \
	sudo mount $$LOOP"p1" /tmp/efimount; \
	sudo cp -r test/. /tmp/efimount; \
	sync; \
	sudo umount /tmp/efimount; \
	sudo losetup -d $$LOOP

clean:
	rm -rf $(TARGET) $(DISK)
.PHONY: clean run