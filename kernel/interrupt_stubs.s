.section .text

.extern interrupts_dispatch

.global irq12_stub
.type irq12_stub, @function

irq12_stub:
    pusha

    pushl $44
    call interrupts_dispatch
    addl $4, %esp

    popa

    movb $0x20, %al
    outb %al, $0xA0

    movb $0x20, %al
    outb %al, $0x20

    iret

.size irq12_stub, .-irq12_stub