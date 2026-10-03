#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed)) IDTEntry;

typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) IDTPointer;

void idt_initialize(void);

void idt_set_gate(
    uint8_t vector,
    uint32_t handler,
    uint16_t selector,
    uint8_t type_attributes
);

void idt_load(void);

#endif