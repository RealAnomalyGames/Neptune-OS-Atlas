#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void keyboard_initialize(void);
uint8_t keyboard_read_scancode(void);
int keyboard_has_input(void);

#endif