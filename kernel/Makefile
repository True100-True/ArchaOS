#TARGET = kernel.bin

SRC = simple.c
OBJ = simple.o
ELF = kernel.elf
LNK = linker.ld

LD = x86_64-elf-ld
CC = x86_64-elf-gcc

#$(TARGET): $(ELF)
#	x86_64-elf-objcopy \
#		-O binary \
#		$(ELF) \
#		$(TARGET)

$(ELF): $(OBJ)
	$(LD) \
		-T $(LNK) \
		$(OBJ) \
		-o $(ELF)

$(OBJ): $(SRC)
	$(CC) \
		-ffreestanding \
		-mno-red-zone \
		-mno-sse \
		-mno-sse2 \
		-c $(SRC) \
		-o $(OBJ)

clean:
	rm -f $(OBJ) $(ELF) $(TARGET)

.PHONY: clean