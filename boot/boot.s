.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16

stack_bottom:
.skip 16384

stack_top:

.section .text
.global _start
.type _start, @function

_start:
    cli

    # Set up the kernel stack.
    mov $stack_top, %esp

    # GRUB places the Multiboot information address in EBX.
    # Pass it as the first argument to kernel_main().
    push %ebx

    call kernel_main

hang:
    cli
    hlt
    jmp hang

.size _start, . - _start