#include "keyboard.h"
#include "io.h"

void keyboard_initialize(void)
{
    /*
     * The PS/2 controller requires no special initialization
     * for our basic polling implementation.
     */
}

int keyboard_has_input(void)
{
    uint8_t status = inb(KEYBOARD_STATUS_PORT);

    return status & 0x01;
}

uint8_t keyboard_read_scancode(void)
{
    return inb(KEYBOARD_DATA_PORT);
}