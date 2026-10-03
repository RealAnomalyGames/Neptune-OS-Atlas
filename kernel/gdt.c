#include "gdt.h"

typedef struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDTEntry;

typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) GDTPointer;

static GDTEntry gdt[3];
static GDTPointer gdt_pointer;

static void gdt_set_gate(
    uint8_t index,
    uint32_t base,
    uint32_t limit,
    uint8_t access,
    uint8_t granularity
)
{
    gdt[index].base_low =
        (uint16_t)(base & 0xFFFF);

    gdt[index].base_middle =
        (uint8_t)((base >> 16) & 0xFF);

    gdt[index].base_high =
        (uint8_t)((base >> 24) & 0xFF);

    gdt[index].limit_low =
        (uint16_t)(limit & 0xFFFF);

    gdt[index].granularity =
        (uint8_t)((limit >> 16) & 0x0F);

    gdt[index].granularity |=
        granularity & 0xF0;

    gdt[index].access = access;
}

void gdt_initialize(void)
{
    gdt_pointer.limit =
        sizeof(gdt) - 1;

    gdt_pointer.base =
        (uint32_t)&gdt;

    /*
     * Null descriptor.
     */
    gdt_set_gate(
        0,
        0,
        0,
        0,
        0
    );

    /*
     * Kernel code segment.
     * Selector: 0x08
     */
    gdt_set_gate(
        1,
        0,
        0xFFFFFFFF,
        0x9A,
        0xCF
    );

    /*
     * Kernel data segment.
     * Selector: 0x10
     */
    gdt_set_gate(
        2,
        0,
        0xFFFFFFFF,
        0x92,
        0xCF
    );

    __asm__ volatile (
        "lgdt %0"
        :
        : "m"(gdt_pointer)
    );

    /*
     * Reload the code segment.
     */
    __asm__ volatile (
        "ljmp $0x08, $1f\n"
        "1:\n"
        :
        :
        : "memory"
    );

    /*
     * Reload the data segments.
     */
    __asm__ volatile (
        "movw $0x10, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        :
        :
        : "ax", "memory"
    );
}