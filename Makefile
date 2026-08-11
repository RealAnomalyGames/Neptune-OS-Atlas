ISO = build/neptune-atlas-build003.iso
ISO_DIR = build/isodir

iso: build/kernel.bin
	mkdir -p $(ISO_DIR)/boot/grub
	cp build/kernel.bin $(ISO_DIR)/boot/kernel.bin
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR)

CC = gcc
AS = as
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall
ASFLAGS = --32
LDFLAGS = -m elf_i386 -T kernel/linker.ld

BUILD = build

KERNEL_OBJECTS = \
	$(BUILD)/kernel.o \
	$(BUILD)/keyboard.o \
	$(BUILD)/terminal.o \
	$(BUILD)/io.o \
	$(BUILD)/parser.o \
	$(BUILD)/shell.o

all: $(BUILD)/kernel.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot/boot.s | $(BUILD)
	$(AS) $(ASFLAGS) boot/boot.s -o $(BUILD)/boot.o

$(BUILD)/kernel.o: kernel/kernel.c kernel/keyboard.h kernel/terminal.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/keyboard.o: kernel/keyboard.c kernel/keyboard.h kernel/io.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/keyboard.c -o $(BUILD)/keyboard.o

$(BUILD)/terminal.o: kernel/terminal.c kernel/terminal.h kernel/io.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/terminal.c -o $(BUILD)/terminal.o

$(BUILD)/io.o: kernel/io.c kernel/io.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/io.c -o $(BUILD)/io.o

$(BUILD)/shell.o: kernel/shell.c kernel/shell.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/shell.c -o $(BUILD)/shell.o

$(BUILD)/parser.o: kernel/parser.c kernel/parser.h | $(BUILD)
	$(CC) $(CFLAGS) -c kernel/parser.c -o $(BUILD)/parser.o

$(BUILD)/kernel.bin: $(BUILD)/boot.o $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -o $(BUILD)/kernel.bin \
		$(BUILD)/boot.o $(KERNEL_OBJECTS)

clean:
	rm -rf $(BUILD)

.PHONY: all clean iso