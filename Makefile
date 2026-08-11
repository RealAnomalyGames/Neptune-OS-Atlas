TARGET = neptune-os-atlas-build002.iso
KERNEL = kernel.bin

CC = gcc
AS = as
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -T linker.ld

BUILD = build
ISO = $(BUILD)/iso

.PHONY: all clean run

all: $(TARGET)

boot.o: boot/boot.s
	$(AS) --32 $< -o $@

kernel.o: kernel/kernel.c kernel/kernel.h kernel/terminal.h
	$(CC) $(CFLAGS) -c kernel/kernel.c -o $@

terminal.o: kernel/terminal.c kernel/terminal.h
	$(CC) $(CFLAGS) -c kernel/terminal.c -o $@

$(KERNEL): boot.o kernel.o terminal.o linker.ld
	$(LD) $(LDFLAGS) -o $@ boot.o kernel.o terminal.o

$(TARGET): $(KERNEL) grub/grub.cfg
	mkdir -p $(ISO)/boot/grub
	cp $(KERNEL) $(ISO)/boot/kernel.bin
	cp grub/grub.cfg $(ISO)/boot/grub/grub.cfg
	grub-file --is-x86-multiboot $(KERNEL)
	grub-mkrescue -o $(TARGET) $(ISO)

run: $(TARGET)
	qemu-system-i386 -cdrom $(TARGET) -display none -serial stdio

clean:
	rm -rf $(BUILD) $(TARGET) $(KERNEL) boot.o kernel.o terminal.o