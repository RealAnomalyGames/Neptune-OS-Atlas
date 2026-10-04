#include "graphics.h"
#include "io.h"

#define VGA_MISC_OUTPUT_WRITE 0x3C2

#define VGA_SEQ_INDEX         0x3C4
#define VGA_SEQ_DATA          0x3C5

#define VGA_CRTC_INDEX        0x3D4
#define VGA_CRTC_DATA         0x3D5

#define VGA_GC_INDEX          0x3CE
#define VGA_GC_DATA           0x3CF

#define VGA_AC_INDEX          0x3C0
#define VGA_AC_READ           0x3C1

#define VGA_INPUT_STATUS      0x3DA

#define VGA_FRAMEBUFFER       ((uint8_t*)0xA0000)

static uint8_t graphics_back_buffer[
    GRAPHICS_WIDTH * GRAPHICS_HEIGHT
];

#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9

static const uint8_t vga_sequencer_registers[] =
{
    0x03,
    0x01,
    0x0F,
    0x00,
    0x0E
};

static const uint8_t vga_crtc_registers[] =
{
    0x5F,
    0x4F,
    0x50,
    0x82,
    0x54,
    0x80,
    0xBF,
    0x1F,
    0x00,
    0x41,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x9C,
    0x0E,
    0x8F,
    0x28,
    0x40,
    0x96,
    0xB9,
    0xA3,
    0xFF
};

static const uint8_t vga_graphics_controller_registers[] =
{
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x40,
    0x05,
    0x0F,
    0xFF
};

static const uint8_t vga_attribute_controller_registers[] =
{
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x0F,
    0x41,
    0x00,
    0x0F,
    0x00,
    0x00
};

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} GraphicsColor;

static const GraphicsColor atlas_palette[] =
{
    {  0,  0,  0 },   /* Black */
    {  0,  0, 42 },   /* Blue */
    {  0,  0, 20 },   /* Dark blue */
    {  0, 42, 42 },   /* Cyan */
    { 63, 63, 63 },   /* White */
    { 42, 42, 42 },   /* Gray */
    { 20, 20, 20 },   /* Dark gray */
    { 42,  0,  0 },   /* Red */
    {  0, 42,  0 },   /* Green */
    { 42, 42,  0 }    /* Yellow */
};

static const uint8_t bitmap_font[95][8] =
{
    /* 32: Space */
    {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },

    /* 33: ! */
    {
        0x18, 0x18, 0x18, 0x18,
        0x18, 0x00, 0x18, 0x00
    },

    /* 34: " */
    {
        0x66, 0x66, 0x24, 0x00,
        0x00, 0x00, 0x00, 0x00
    },

    /* 35: # */
    {
        0x24, 0x24, 0x7E, 0x24,
        0x7E, 0x24, 0x24, 0x00
    },

    /* 36: $ */
    {
        0x18, 0x3E, 0x60, 0x3C,
        0x06, 0x7C, 0x18, 0x00
    },

    /* 37: % */
    {
        0x62, 0x64, 0x08, 0x10,
        0x26, 0x46, 0x00, 0x00
    },

    /* 38: & */
    {
        0x30, 0x48, 0x30, 0x4A,
        0x44, 0x3A, 0x00, 0x00
    },

    /* 39: ' */
    {
        0x18, 0x18, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00
    },

    /* 40: ( */
    {
        0x0C, 0x18, 0x30, 0x30,
        0x30, 0x18, 0x0C, 0x00
    },

    /* 41: ) */
    {
        0x30, 0x18, 0x0C, 0x0C,
        0x0C, 0x18, 0x30, 0x00
    },

    /* 42: * */
    {
        0x00, 0x24, 0x18, 0x7E,
        0x18, 0x24, 0x00, 0x00
    },

    /* 43: + */
    {
        0x00, 0x18, 0x18, 0x7E,
        0x18, 0x18, 0x00, 0x00
    },

    /* 44: , */
    {
        0x00, 0x00, 0x00, 0x00,
        0x18, 0x18, 0x10, 0x00
    },

    /* 45: - */
    {
        0x00, 0x00, 0x00, 0x7E,
        0x00, 0x00, 0x00, 0x00
    },

    /* 46: . */
    {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x18, 0x18, 0x00
    },

    /* 47: / */
    {
        0x02, 0x04, 0x08, 0x10,
        0x20, 0x40, 0x00, 0x00
    },

    /* 48-57: 0-9 */
    {
        0x3C, 0x66, 0x6E, 0x76,
        0x66, 0x66, 0x3C, 0x00
    },
    {
        0x18, 0x38, 0x18, 0x18,
        0x18, 0x18, 0x7E, 0x00
    },
    {
        0x3C, 0x66, 0x06, 0x0C,
        0x18, 0x30, 0x7E, 0x00
    },
    {
        0x3C, 0x66, 0x06, 0x1C,
        0x06, 0x66, 0x3C, 0x00
    },
    {
        0x0C, 0x1C, 0x2C, 0x4C,
        0x7E, 0x0C, 0x0C, 0x00
    },
    {
        0x7E, 0x60, 0x7C, 0x06,
        0x06, 0x66, 0x3C, 0x00
    },
    {
        0x1C, 0x30, 0x60, 0x7C,
        0x66, 0x66, 0x3C, 0x00
    },
    {
        0x7E, 0x06, 0x0C, 0x18,
        0x30, 0x30, 0x30, 0x00
    },
    {
        0x3C, 0x66, 0x66, 0x3C,
        0x66, 0x66, 0x3C, 0x00
    },
    {
        0x3C, 0x66, 0x66, 0x3E,
        0x06, 0x0C, 0x38, 0x00
    },

    /* 58: : */
    {
        0x00, 0x18, 0x18, 0x00,
        0x00, 0x18, 0x18, 0x00
    },

    /* 59: ; */
    {
        0x00, 0x18, 0x18, 0x00,
        0x00, 0x18, 0x18, 0x10
    },

    /* 60: < */
    {
        0x0C, 0x18, 0x30, 0x60,
        0x30, 0x18, 0x0C, 0x00
    },

    /* 61: = */
    {
        0x00, 0x00, 0x7E, 0x00,
        0x7E, 0x00, 0x00, 0x00
    },

    /* 62: > */
    {
        0x30, 0x18, 0x0C, 0x06,
        0x0C, 0x18, 0x30, 0x00
    },

    /* 63: ? */
    {
        0x3C, 0x66, 0x06, 0x0C,
        0x18, 0x00, 0x18, 0x00
    },

    /* 64: @ */
    {
        0x3C, 0x42, 0x5A, 0x5A,
        0x5C, 0x40, 0x3C, 0x00
    },

    /* 65-90: A-Z */
    {
        0x18, 0x24, 0x42, 0x7E,
        0x42, 0x42, 0x42, 0x00
    },
    {
        0x7C, 0x42, 0x42, 0x7C,
        0x42, 0x42, 0x7C, 0x00
    },
    {
        0x3C, 0x42, 0x40, 0x40,
        0x40, 0x42, 0x3C, 0x00
    },
    {
        0x78, 0x44, 0x42, 0x42,
        0x42, 0x44, 0x78, 0x00
    },
    {
        0x7E, 0x40, 0x40, 0x7C,
        0x40, 0x40, 0x7E, 0x00
    },
    {
        0x7E, 0x40, 0x40, 0x7C,
        0x40, 0x40, 0x40, 0x00
    },
    {
        0x3C, 0x42, 0x40, 0x4E,
        0x42, 0x42, 0x3C, 0x00
    },
    {
        0x42, 0x42, 0x42, 0x7E,
        0x42, 0x42, 0x42, 0x00
    },
    {
        0x7E, 0x18, 0x18, 0x18,
        0x18, 0x18, 0x7E, 0x00
    },
    {
        0x1E, 0x04, 0x04, 0x04,
        0x04, 0x44, 0x38, 0x00
    },
    {
        0x42, 0x44, 0x48, 0x70,
        0x48, 0x44, 0x42, 0x00
    },
    {
        0x40, 0x40, 0x40, 0x40,
        0x40, 0x40, 0x7E, 0x00
    },
    {
        0x42, 0x66, 0x5A, 0x5A,
        0x42, 0x42, 0x42, 0x00
    },
    {
        0x42, 0x62, 0x52, 0x4A,
        0x46, 0x42, 0x42, 0x00
    },
    {
        0x3C, 0x42, 0x42, 0x42,
        0x42, 0x42, 0x3C, 0x00
    },
    {
        0x7C, 0x42, 0x42, 0x7C,
        0x40, 0x40, 0x40, 0x00
    },
    {
        0x3C, 0x42, 0x42, 0x42,
        0x4A, 0x44, 0x3A, 0x00
    },
    {
        0x7C, 0x42, 0x42, 0x7C,
        0x48, 0x44, 0x42, 0x00
    },
    {
        0x3C, 0x42, 0x40, 0x3C,
        0x02, 0x42, 0x3C, 0x00
    },
    {
        0x7E, 0x18, 0x18, 0x18,
        0x18, 0x18, 0x18, 0x00
    },
    {
        0x42, 0x42, 0x42, 0x42,
        0x42, 0x42, 0x3C, 0x00
    },
    {
        0x42, 0x42, 0x42, 0x42,
        0x42, 0x24, 0x18, 0x00
    },
    {
        0x42, 0x42, 0x42, 0x5A,
        0x5A, 0x66, 0x42, 0x00
    },
    {
        0x42, 0x42, 0x24, 0x18,
        0x24, 0x42, 0x42, 0x00
    },
    {
        0x42, 0x42, 0x24, 0x18,
        0x18, 0x18, 0x18, 0x00
    },
    {
        0x7E, 0x02, 0x04, 0x18,
        0x20, 0x40, 0x7E, 0x00
    },

    /* 91: [ */
    {
        0x3C, 0x30, 0x30, 0x30,
        0x30, 0x30, 0x3C, 0x00
    },

    /* 92: \ */
    {
        0x40, 0x20, 0x10, 0x08,
        0x04, 0x02, 0x00, 0x00
    },

    /* 93: ] */
    {
        0x3C, 0x0C, 0x0C, 0x0C,
        0x0C, 0x0C, 0x3C, 0x00
    },

    /* 94: ^ */
    {
        0x18, 0x24, 0x42, 0x00,
        0x00, 0x00, 0x00, 0x00
    },

    /* 95: _ */
    {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x7E, 0x00
    },

    /* 96: ` */
    {
        0x30, 0x18, 0x0C, 0x00,
        0x00, 0x00, 0x00, 0x00
    },

    /* 97-122: a-z */
    {
        0x00, 0x00, 0x3C, 0x02,
        0x3E, 0x46, 0x3A, 0x00
    },
    {
        0x40, 0x40, 0x5C, 0x62,
        0x42, 0x62, 0x5C, 0x00
    },
    {
        0x00, 0x00, 0x3C, 0x42,
        0x40, 0x42, 0x3C, 0x00
    },
    {
        0x02, 0x02, 0x3A, 0x46,
        0x42, 0x46, 0x3A, 0x00
    },
    {
        0x00, 0x00, 0x3C, 0x42,
        0x7E, 0x40, 0x3C, 0x00
    },
    {
        0x0E, 0x10, 0x3C, 0x10,
        0x10, 0x10, 0x10, 0x00
    },
    {
        0x00, 0x00, 0x3A, 0x46,
        0x46, 0x3A, 0x02, 0x3C
    },
    {
        0x40, 0x40, 0x5C, 0x62,
        0x42, 0x42, 0x42, 0x00
    },
    {
        0x18, 0x00, 0x18, 0x18,
        0x18, 0x18, 0x3C, 0x00
    },
    {
        0x06, 0x00, 0x06, 0x06,
        0x06, 0x46, 0x46, 0x3C
    },
    {
        0x40, 0x40, 0x44, 0x48,
        0x70, 0x48, 0x44, 0x00
    },
    {
        0x18, 0x18, 0x18, 0x18,
        0x18, 0x18, 0x0E, 0x00
    },
    {
        0x00, 0x00, 0x66, 0x7E,
        0x5A, 0x5A, 0x42, 0x00
    },
    {
        0x00, 0x00, 0x5C, 0x62,
        0x42, 0x42, 0x42, 0x00
    },
    {
        0x00, 0x00, 0x3C, 0x42,
        0x42, 0x42, 0x3C, 0x00
    },
    {
        0x00, 0x00, 0x5C, 0x62,
        0x62, 0x5C, 0x40, 0x40
    },
    {
        0x00, 0x00, 0x3A, 0x46,
        0x46, 0x3A, 0x02, 0x02
    },
    {
        0x00, 0x00, 0x5C, 0x62,
        0x40, 0x40, 0x40, 0x00
    },
    {
        0x00, 0x00, 0x3E, 0x40,
        0x3C, 0x02, 0x7C, 0x00
    },
    {
        0x10, 0x10, 0x7C, 0x10,
        0x10, 0x12, 0x0C, 0x00
    },
    {
        0x00, 0x00, 0x42, 0x42,
        0x42, 0x46, 0x3A, 0x00
    },
    {
        0x00, 0x00, 0x42, 0x42,
        0x42, 0x24, 0x18, 0x00
    },
    {
        0x00, 0x00, 0x42, 0x5A,
        0x5A, 0x66, 0x42, 0x00
    },
    {
        0x00, 0x00, 0x42, 0x24,
        0x18, 0x24, 0x42, 0x00
    },
    {
        0x00, 0x00, 0x42, 0x42,
        0x46, 0x3A, 0x02, 0x3C
    },
    {
        0x00, 0x00, 0x7E, 0x04,
        0x18, 0x20, 0x7E, 0x00
    },

    /* 123: { */
    {
        0x0C, 0x18, 0x18, 0x70,
        0x18, 0x18, 0x0C, 0x00
    },

    /* 124: | */
    {
        0x18, 0x18, 0x18, 0x18,
        0x18, 0x18, 0x18, 0x00
    },

    /* 125: } */
    {
        0x30, 0x18, 0x18, 0x0E,
        0x18, 0x18, 0x30, 0x00
    },

    /* 126: ~ */
    {
        0x00, 0x00, 0x32, 0x4C,
        0x00, 0x00, 0x00, 0x00
    }
};

static const uint8_t* graphics_get_font(char character)
{
    if (
        character < 32 ||
        character > 126
    )
    {
        return bitmap_font[0];
    }

    return bitmap_font[
        (uint8_t)character - 32
    ];
}

static void vga_write_sequencer(
    uint8_t index,
    uint8_t value
)
{
    outb(VGA_SEQ_INDEX, index);
    outb(VGA_SEQ_DATA, value);
}

static void vga_write_crtc(
    uint8_t index,
    uint8_t value
)
{
    outb(VGA_CRTC_INDEX, index);
    outb(VGA_CRTC_DATA, value);
}

static void vga_write_graphics_controller(
    uint8_t index,
    uint8_t value
)
{
    outb(VGA_GC_INDEX, index);
    outb(VGA_GC_DATA, value);
}

static void vga_write_attribute_controller(
    uint8_t index,
    uint8_t value
)
{
    inb(VGA_INPUT_STATUS);

    outb(VGA_AC_INDEX, index);
    outb(VGA_AC_INDEX, value);
}

static void graphics_initialize_palette(void)
{
    uint32_t i;
    uint32_t count;

    count =
        sizeof(atlas_palette) /
        sizeof(atlas_palette[0]);

    for (i = 0; i < count; i++)
    {
        outb(
            VGA_DAC_WRITE_INDEX,
            (uint8_t)i
        );

        outb(
            VGA_DAC_DATA,
            atlas_palette[i].red
        );

        outb(
            VGA_DAC_DATA,
            atlas_palette[i].green
        );

        outb(
            VGA_DAC_DATA,
            atlas_palette[i].blue
        );
    }
}

void graphics_set_mode_13h(void)
{
    uint32_t i;

    /*
     * Miscellaneous Output Register.
     *
     * Selects the 25.175 MHz VGA clock and
     * enables color I/O addresses.
     */
    outb(VGA_MISC_OUTPUT_WRITE, 0x63);

    /*
     * Unlock CRTC registers 0-7.
     */
    outb(VGA_CRTC_INDEX, 0x03);
    outb(
        VGA_CRTC_DATA,
        inb(VGA_CRTC_DATA) | 0x80
    );

    outb(VGA_CRTC_INDEX, 0x11);
    outb(
        VGA_CRTC_DATA,
        inb(VGA_CRTC_DATA) & 0x7F
    );

    /*
     * Sequencer registers.
     */
    for (i = 0; i < 5; i++)
    {
        vga_write_sequencer(
            (uint8_t)i,
            vga_sequencer_registers[i]
        );
    }

    /*
     * CRTC registers.
     */
    for (i = 0; i < 25; i++)
    {
        vga_write_crtc(
            (uint8_t)i,
            vga_crtc_registers[i]
        );
    }

    /*
     * Graphics Controller registers.
     */
    for (i = 0; i < 9; i++)
    {
        vga_write_graphics_controller(
            (uint8_t)i,
            vga_graphics_controller_registers[i]
        );
    }

    /*
     * Attribute Controller registers.
     */
    for (i = 0; i < 21; i++)
    {
        vga_write_attribute_controller(
            (uint8_t)i,
            vga_attribute_controller_registers[i]
        );
    }

    /*
     * Return the Attribute Controller to
     * normal display mode.
     */
    inb(VGA_INPUT_STATUS);
    outb(VGA_AC_INDEX, 0x20);
}

void graphics_initialize(void)
{
    graphics_set_mode_13h();
    graphics_initialize_palette();
}

void graphics_clear(uint8_t color)
{
    uint32_t i;

    for (
        i = 0;
        i < GRAPHICS_WIDTH * GRAPHICS_HEIGHT;
        i++
    )
    {
        graphics_back_buffer[i] = color;
    }
}

void graphics_present(void)
{
    uint32_t i;

    for (
        i = 0;
        i < GRAPHICS_WIDTH * GRAPHICS_HEIGHT;
        i++
    )
    {
        VGA_FRAMEBUFFER[i] =
            graphics_back_buffer[i];
    }
}

void graphics_put_pixel(
    uint16_t x,
    uint16_t y,
    uint8_t color
)
{
    uint32_t offset;

    if (x >= GRAPHICS_WIDTH)
    {
        return;
    }

    if (y >= GRAPHICS_HEIGHT)
    {
        return;
    }

    offset =
        ((uint32_t)y * GRAPHICS_WIDTH) + x;

    graphics_back_buffer[offset] = color;
}

void graphics_draw_char(
    uint16_t x,
    uint16_t y,
    char character,
    uint8_t color
)
{
    const uint8_t* glyph;
    uint8_t row;
    uint8_t column;

    glyph = graphics_get_font(character);

    for (row = 0; row < 8; row++)
    {
        for (column = 0; column < 8; column++)
        {
            if (glyph[row] & (1 << (7 - column)))
            {
                graphics_put_pixel(
                    x + column,
                    y + row,
                    color
                );
            }
        }
    }
}

void graphics_draw_text(
    uint16_t x,
    uint16_t y,
    const char* text,
    uint8_t color
)
{
    uint16_t current_x = x;

    while (*text != '\0')
    {
        graphics_draw_char(
            current_x,
            y,
            *text,
            color
        );

        current_x += 8;
        text++;
    }
}

void graphics_draw_cursor(
    int32_t x,
    int32_t y
)
{
    static const uint8_t cursor_bitmap[10] =
    {
        0x80,
        0xC0,
        0xE0,
        0xF0,
        0xF8,
        0xFC,
        0xFE,
        0xFF,
        0xF8,
        0x18
    };

    uint8_t row;
    uint8_t column;

    for (row = 0; row < 10; row++)
    {
        for (column = 0; column < 8; column++)
        {
            if (
                cursor_bitmap[row] &
                (1 << (7 - column))
            )
            {
                graphics_put_pixel(
                    (uint16_t)(x + column),
                    (uint16_t)(y + row),
                    ATLAS_COLOR_WHITE
                );
            }
        }
    }
}

void graphics_draw_line(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    uint8_t color
)
{
    int32_t dx;
    int32_t dy;
    int32_t sx;
    int32_t sy;
    int32_t error;
    int32_t error2;

    int32_t x = x1;
    int32_t y = y1;

    dx = (x2 > x1) ?
        (int32_t)x2 - x1 :
        (int32_t)x1 - x2;

    dy = (y2 > y1) ?
        (int32_t)y2 - y1 :
        (int32_t)y1 - y2;

    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;

    error = dx - dy;

    while (1)
    {
        graphics_put_pixel(
            (uint16_t)x,
            (uint16_t)y,
            color
        );

        if (x == x2 && y == y2)
        {
            break;
        }

        error2 = error * 2;

        if (error2 > -dy)
        {
            error -= dy;
            x += sx;
        }

        if (error2 < dx)
        {
            error += dx;
            y += sy;
        }
    }
}

void graphics_draw_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint8_t color
)
{
    uint16_t i;

    if (width == 0 || height == 0)
    {
        return;
    }

    for (i = 0; i < width; i++)
    {
        graphics_put_pixel(
            x + i,
            y,
            color
        );

        graphics_put_pixel(
            x + i,
            y + height - 1,
            color
        );
    }

    for (i = 0; i < height; i++)
    {
        graphics_put_pixel(
            x,
            y + i,
            color
        );

        graphics_put_pixel(
            x + width - 1,
            y + i,
            color
        );
    }
}

void graphics_fill_rect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint8_t color
)
{
    uint16_t current_y;
    uint16_t current_x;

    if (width == 0 || height == 0)
    {
        return;
    }

    for (current_y = 0; current_y < height; current_y++)
    {
        for (current_x = 0; current_x < width; current_x++)
        {
            graphics_put_pixel(
                x + current_x,
                y + current_y,
                color
            );
        }
    }
}