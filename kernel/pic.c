#include "pic.h"
#include "io.h"

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA    0x21

#define PIC_SLAVE_COMMAND  0xA0
#define PIC_SLAVE_DATA     0xA1

#define PIC_EOI            0x20

void pic_initialize(void)
{
    /*
     * Start initialization sequence.
     */
    outb(PIC_MASTER_COMMAND, 0x11);
    outb(PIC_SLAVE_COMMAND, 0x11);

    /*
     * Remap IRQs:
     *
     * Master: 32-39
     * Slave:  40-47
     */
    outb(PIC_MASTER_DATA, 0x20);
    outb(PIC_SLAVE_DATA, 0x28);

    /*
     * Tell master about slave on IRQ2.
     */
    outb(PIC_MASTER_DATA, 0x04);

    /*
     * Tell slave its cascade identity.
     */
    outb(PIC_SLAVE_DATA, 0x02);

    /*
     * 8086/88 mode.
     */
    outb(PIC_MASTER_DATA, 0x01);
    outb(PIC_SLAVE_DATA, 0x01);

    /*
     * Initially mask all hardware IRQs.
     */
    outb(PIC_MASTER_DATA, 0xFF);
    outb(PIC_SLAVE_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
    {
        outb(PIC_SLAVE_COMMAND, PIC_EOI);
    }

    outb(PIC_MASTER_COMMAND, PIC_EOI);
}

void pic_unmask_irq(uint8_t irq)
{
    uint8_t mask;

    if (irq < 8)
    {
        mask = inb(PIC_MASTER_DATA);
        mask &= (uint8_t)~(1 << irq);
        outb(PIC_MASTER_DATA, mask);
    }
    else
    {
        mask = inb(PIC_SLAVE_DATA);
        mask &= (uint8_t)~(1 << (irq - 8));
        outb(PIC_SLAVE_DATA, mask);

        /*
         * IRQ2 on the master connects the slave PIC.
         */
        mask = inb(PIC_MASTER_DATA);
        mask &= (uint8_t)~(1 << 2);
        outb(PIC_MASTER_DATA, mask);
    }
}

void pic_mask_irq(uint8_t irq)
{
    uint8_t mask;

    if (irq < 8)
    {
        mask = inb(PIC_MASTER_DATA);
        mask |= (uint8_t)(1 << irq);
        outb(PIC_MASTER_DATA, mask);
    }
    else
    {
        mask = inb(PIC_SLAVE_DATA);
        mask |= (uint8_t)(1 << (irq - 8));
        outb(PIC_SLAVE_DATA, mask);
    }
}