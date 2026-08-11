#include "keyboard.h"
#include "io.h"

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int caps_lock = 0;

static const char keyboard_map[128] =
{
    0,
    0,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t',

    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', '\n', 0,

    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`', 0, '\\',

    'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '/', 0, '*', 0, ' ',

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static const char keyboard_shift_map[128] =
{
    0,
    0,
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t',

    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}', '\n', 0,

    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"', '~', 0, '|',

    'Z', 'X', 'C', 'V', 'B', 'N', 'M',
    '<', '>', '?', 0, '*', 0, ' ',

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void keyboard_initialize(void)
{
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    caps_lock = 0;
}

int keyboard_has_input(void)
{
    uint8_t status = inb(KEYBOARD_STATUS_PORT);

    return status & 0x01;
}

uint8_t keyboard_read_scancode(void)
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    keyboard_update_state(scancode);

    return scancode;
}

void keyboard_process_scancode(uint8_t scancode)
{
    int released = scancode & SCANCODE_RELEASE;
    uint8_t key = scancode & 0x7F;

    if (key == SCANCODE_LEFT_SHIFT ||
        key == SCANCODE_RIGHT_SHIFT)
    {
        shift_pressed = !released;
        return;
    }

    if (key == SCANCODE_LEFT_CTRL)
    {
        ctrl_pressed = !released;
        return;
    }

    if (key == SCANCODE_LEFT_ALT)
    {
        alt_pressed = !released;
        return;
    }

    if (key == SCANCODE_CAPS_LOCK && !released)
    {
        caps_lock = !caps_lock;
        return;
    }
}

int keyboard_shift_pressed(void)
{
    return shift_pressed;
}

int keyboard_ctrl_pressed(void)
{
    return ctrl_pressed;
}

int keyboard_alt_pressed(void)
{
    return alt_pressed;
}

int keyboard_caps_lock(void)
{
    return caps_lock;
}