CC = x86_64-w64-mingw32-gcc

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -nostdlib \
	-Wall -Wextra -Wpedantic \
	-Wl,-subsystem,10 -Wl,--entry=efi_main

TARGET = BOOTX64.EFI
SOURCE = uefi.c
DISK = img.img

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<


$(DISK): $(TARGET)
	cp $(TARGET) ../test_disk/EFI/BOOT/
	cp ../kernel/kernel.elf ../test_disk/

	rm -f $(DISK)

	dd if=/dev/zero of=$(DISK) bs=1M count=64

	parted $(DISK) --script \
		mklabel gpt \
		mkpart ESP fat32 1MiB 63MiB \
		set 1 esp on

	sudo losetup -Pf $(DISK)

	LOOP=$$(losetup -j $(DISK) | cut -d: -f1); \
	echo "Using $$LOOP"; \
	sudo mkfs.fat -F32 -n ESP $$LOOP"p1"; \
	sudo mkdir -p /tmp/efimount; \
	sudo mount $$LOOP"p1" /tmp/efimount; \
	sudo cp -r ../test_disk/. /tmp/efimount; \
	sync; \
	sudo umount /tmp/efimount; \
	sudo losetup -d $$LOOP


run: $(DISK)
	qemu-system-x86_64 \
		-machine q35 \
		-m 512M \
		-cpu qemu64,vendor="AuthenticAMD" \
		-bios /usr/share/ovmf/OVMF.fd \
		-drive file=$(DISK),format=raw \
		-net none \
		-serial stdio \
		-device virtio-gpu \
		-display gtk

# -device VGA,edid=off \


clean:
	rm -f $(TARGET) $(DISK)

.PHONY: all run clean