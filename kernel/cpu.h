#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define CPU_VENDOR_LENGTH 13

typedef struct
{
    char vendor[CPU_VENDOR_LENGTH];

    uint32_t maximum_basic_leaf;
    uint32_t maximum_extended_leaf;

    uint8_t cpuid_supported;

} CPUInformation;

void cpu_initialize(void);

void cpu_get_information(CPUInformation* information);

const char* cpu_get_vendor(void);

uint8_t cpu_is_supported(void);

#endif