#include "idt.h"

#define IDT_ENTRIES 256

static IDTEntry idt[IDT_ENTRIES];
static IDTPointer idt_pointer;

void idt_set_gate(
    uint8_t vector,
    uint32_t handler,
    uint16_t selector,
    uint8_t type_attributes
)
{
    idt[vector].offset_low = (uint16_t)(handler & 0xFFFF);
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attributes = type_attributes;
    idt[vector].offset_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

void idt_load(void)
{
    __asm__ volatile (
        "lidt %0"
        :
        : "m"(idt_pointer)
    );
}

void idt_initialize(void)
{
    uint32_t i;

    for (i = 0; i < IDT_ENTRIES; i++)
    {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attributes = 0;
        idt[i].offset_high = 0;
    }

    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = (uint32_t)&idt;

    idt_load();
}