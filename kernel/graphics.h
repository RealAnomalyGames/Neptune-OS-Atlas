#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#define GRAPHICS_WIDTH  320
#define GRAPHICS_HEIGHT 200

#define ATLAS_COLOR_BLACK       0
#define ATLAS_COLOR_BLUE       1
#define ATLAS_COLOR_DARK_BLUE  2
#define ATLAS_COLOR_CYAN       3
#define ATLAS_COLOR_WHITE      4
#define ATLAS_COLOR_GRAY       5
#define ATLAS_COLOR_DARK_GRAY  6
#define ATLAS_COLOR_RED        7
#define ATLAS_COLOR_GREEN      8
#define ATLAS_COLOR_YELLOW     9

void graphics_initialize(void);

void graphics_present(void);

void graphics_set_mode_13h(void);

void graphics_clear(uint8_t color);

void graphics_put_pixel(
    uint16_t x,
    uint16_t y,
    uint8_t color
);

void graphics_draw_line(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    uint8_t color
);

void graphics_draw_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint8_t color
);

void graphics_fill_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint8_t color
);

void graphics_draw_char(
    uint16_t x,
    uint16_t y,
    char character,
    uint8_t color
);

void graphics_draw_text(
    uint16_t x,
    uint16_t y,
    const char* text,
    uint8_t color
);

void graphics_draw_cursor(
    int32_t x,
    int32_t y
);

#endif