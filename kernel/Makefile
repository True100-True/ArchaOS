CC=x86_64-elf-gcc
LD=x86_64-elf-ld

CFLAGS= \
    -O2 \
    -ffreestanding \
    -mno-red-zone \
    -mno-sse \
    -mno-sse2 \
    -nostdlib \
    -fno-omit-frame-pointer \
    -Wall \
    -Wextra

LDFLAGS=-T linker.ld

SRC_C := $(shell find . -name "*.c")
SRC_S := $(shell find . -name "*.s")

OBJ := $(SRC_C:.c=.o) $(SRC_S:.s=.o)

ELF=kernel.elf


$(ELF): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


%.o: %.s
	$(CC) -c $< -o $@


clean:
	rm -f $(OBJ) $(ELF)

.PHONY: clean