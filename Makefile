AS = as
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall
LDFLAGS = -m elf_i386 -T kernel/linker.ld

BUILD_DIR = build

KERNEL_OBJECTS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/terminal.o \
	$(BUILD_DIR)/io.o \
	$(BUILD_DIR)/parser.o \
	$(BUILD_DIR)/shell.o \
	$(BUILD_DIR)/system.o \
	$(BUILD_DIR)/cpu.o

KERNEL = $(BUILD_DIR)/kernel.bin

ISO_DIR = $(BUILD_DIR)/iso
ISO = $(BUILD_DIR)/neptune-os-atlas.iso

all: $(KERNEL)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot/boot.s | $(BUILD_DIR)
	$(AS) --32 boot/boot.s -o $(BUILD_DIR)/boot.o

$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $(BUILD_DIR)/kernel.o

$(BUILD_DIR)/keyboard.o: kernel/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/keyboard.c -o $(BUILD_DIR)/keyboard.o

$(BUILD_DIR)/terminal.o: kernel/terminal.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/terminal.c -o $(BUILD_DIR)/terminal.o

$(BUILD_DIR)/io.o: kernel/io.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/io.c -o $(BUILD_DIR)/io.o

$(BUILD_DIR)/parser.o: kernel/parser.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/parser.c -o $(BUILD_DIR)/parser.o

$(BUILD_DIR)/shell.o: kernel/shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/shell.c -o $(BUILD_DIR)/shell.o

$(BUILD_DIR)/system.o: kernel/system.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/system.c -o $(BUILD_DIR)/system.o

$(BUILD_DIR)/cpu.o: kernel/cpu.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/cpu.c -o $(BUILD_DIR)/cpu.o

$(KERNEL): $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -o $(KERNEL) \
		$(KERNEL_OBJECTS)

iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR)

clean:
	rm -rf $(BUILD_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

.PHONY: all iso clean run