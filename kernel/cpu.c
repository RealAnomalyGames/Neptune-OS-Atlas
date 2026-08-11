#include "cpu.h"

static CPUInformation cpu_information;

static void cpu_cpuid(
    uint32_t leaf,
    uint32_t* eax,
    uint32_t* ebx,
    uint32_t* ecx,
    uint32_t* edx
)
{
    __asm__ volatile (
        "cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "a" (leaf)
    );
}

void cpu_initialize(void)
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    cpu_information.cpuid_supported = 1;

    cpu_cpuid(
        0,
        &eax,
        &ebx,
        &ecx,
        &edx
    );

    cpu_information.maximum_basic_leaf = eax;

    cpu_information.vendor[0] =
        (char)(ebx & 0xFF);

    cpu_information.vendor[1] =
        (char)((ebx >> 8) & 0xFF);

    cpu_information.vendor[2] =
        (char)((ebx >> 16) & 0xFF);

    cpu_information.vendor[3] =
        (char)((ebx >> 24) & 0xFF);

    cpu_information.vendor[4] =
        (char)(edx & 0xFF);

    cpu_information.vendor[5] =
        (char)((edx >> 8) & 0xFF);

    cpu_information.vendor[6] =
        (char)((edx >> 16) & 0xFF);

    cpu_information.vendor[7] =
        (char)((edx >> 24) & 0xFF);

    cpu_information.vendor[8] =
        (char)(ecx & 0xFF);

    cpu_information.vendor[9] =
        (char)((ecx >> 8) & 0xFF);

    cpu_information.vendor[10] =
        (char)((ecx >> 16) & 0xFF);

    cpu_information.vendor[11] =
        (char)((ecx >> 24) & 0xFF);

    cpu_information.vendor[12] = '\0';

    cpu_cpuid(
        0x80000000,
        &eax,
        &ebx,
        &ecx,
        &edx
    );

    cpu_information.maximum_extended_leaf = eax;
}

void cpu_get_information(CPUInformation* information)
{
    if (information == 0)
    {
        return;
    }

    *information = cpu_information;
}

const char* cpu_get_vendor(void)
{
    return cpu_information.vendor;
}

uint8_t cpu_is_supported(void)
{
    return cpu_information.cpuid_supported;
}