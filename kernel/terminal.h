#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 25

void terminal_initialize(void);

void terminal_clear(void);

void terminal_putchar(char character);
void terminal_write(const char* string);

void terminal_backspace(void);

void terminal_newline(void);
void terminal_scroll(void);

uint32_t terminal_get_row(void);
uint32_t terminal_get_column(void);

#endif