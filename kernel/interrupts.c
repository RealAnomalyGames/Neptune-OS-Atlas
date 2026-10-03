#include "interrupts.h"
#include "idt.h"
#include "pic.h"

#define INTERRUPT_HANDLER_COUNT 256

static InterruptHandler handlers[INTERRUPT_HANDLER_COUNT];

extern void irq12_stub(void);

void interrupts_register_handler(
    uint8_t vector,
    InterruptHandler handler
)
{
    handlers[vector] = handler;
}

void interrupts_dispatch(uint8_t vector)
{
    if (handlers[vector] != 0)
    {
        handlers[vector]();
    }
}

void interrupts_initialize(void)
{
    uint32_t i;

    for (i = 0; i < INTERRUPT_HANDLER_COUNT; i++)
    {
        handlers[i] = 0;
    }

    pic_initialize();

    idt_initialize();

    idt_set_gate(
        IRQ12,
        (uint32_t)irq12_stub,
        0x08,
        0x8E
    );
}

void interrupts_enable(void)
{
    __asm__ volatile ("sti");
}

void interrupts_disable(void)
{
    __asm__ volatile ("cli");
}