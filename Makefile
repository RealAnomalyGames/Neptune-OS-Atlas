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
	$(BUILD_DIR)/cpu.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/timer.o \
	$(BUILD_DIR)/interrupts.o \
	$(BUILD_DIR)/disk.o \
	$(BUILD_DIR)/filesystem.o \
	$(BUILD_DIR)/task.o \
	$(BUILD_DIR)/scheduler.o \
	$(BUILD_DIR)/task_switch.o \
	$(BUILD_DIR)/mouse.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/pic.o \
	$(BUILD_DIR)/interrupt_stubs.o \
	$(BUILD_DIR)/gdt.o \
	$(BUILD_DIR)/graphics.o \
	$(BUILD_DIR)/window.o \
	$(BUILD_DIR)/desktop.o \
	$(BUILD_DIR)/taskbar.o

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

$(BUILD_DIR)/memory.o: kernel/memory.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/memory.c -o $(BUILD_DIR)/memory.o

$(BUILD_DIR)/timer.o: kernel/timer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/timer.c -o $(BUILD_DIR)/timer.o

$(BUILD_DIR)/interrupts.o: kernel/interrupts.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/interrupts.c -o $(BUILD_DIR)/interrupts.o

$(BUILD_DIR)/disk.o: kernel/disk.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/disk.c -o $(BUILD_DIR)/disk.o

$(BUILD_DIR)/filesystem.o: kernel/filesystem.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/filesystem.c -o $(BUILD_DIR)/filesystem.o

$(BUILD_DIR)/task.o: kernel/task.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/task.c -o $(BUILD_DIR)/task.o

$(BUILD_DIR)/scheduler.o: kernel/scheduler.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/scheduler.c -o $(BUILD_DIR)/scheduler.o

$(BUILD_DIR)/task_switch.o: kernel/task_switch.s | $(BUILD_DIR)
	$(AS) --32 kernel/task_switch.s -o $(BUILD_DIR)/task_switch.o

$(BUILD_DIR)/mouse.o: kernel/mouse.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/mouse.c -o $(BUILD_DIR)/mouse.o

$(BUILD_DIR)/idt.o: kernel/idt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/idt.c -o $(BUILD_DIR)/idt.o

$(BUILD_DIR)/pic.o: kernel/pic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/pic.c -o $(BUILD_DIR)/pic.o

$(BUILD_DIR)/interrupt_stubs.o: kernel/interrupt_stubs.s | $(BUILD_DIR)
	$(AS) --32 kernel/interrupt_stubs.s -o $(BUILD_DIR)/interrupt_stubs.o

$(BUILD_DIR)/gdt.o: kernel/gdt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/gdt.c -o $(BUILD_DIR)/gdt.o

$(BUILD_DIR)/graphics.o: kernel/graphics.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/graphics.c -o $(BUILD_DIR)/graphics.o

$(BUILD_DIR)/window.o: kernel/window.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/window.c -o $(BUILD_DIR)/window.o

$(BUILD_DIR)/desktop.o: kernel/desktop.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/desktop.c -o $(BUILD_DIR)/desktop.o

$(BUILD_DIR)/taskbar.o: kernel/taskbar.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kernel/taskbar.c -o $(BUILD_DIR)/taskbar.o

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