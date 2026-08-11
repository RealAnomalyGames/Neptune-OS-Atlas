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

static uint16_t keyboard_scancode_to_key(uint8_t scancode)
{
    uint8_t key = scancode & 0x7F;

    if (key >= 128)
    {
        return KEY_NONE;
    }

    switch (key)
    {
        case SCANCODE_ESC:
            return KEY_ESCAPE;

        case SCANCODE_BACKSPACE:
            return KEY_BACKSPACE;

        case SCANCODE_TAB:
            return KEY_TAB;

        case SCANCODE_ENTER:
            return KEY_ENTER;

        case SCANCODE_F1:
            return KEY_F1;

        case SCANCODE_F2:
            return KEY_F2;

        case SCANCODE_F3:
            return KEY_F3;

        case SCANCODE_F4:
            return KEY_F4;

        case SCANCODE_F5:
            return KEY_F5;

        case SCANCODE_F6:
            return KEY_F6;

        case SCANCODE_F7:
            return KEY_F7;

        case SCANCODE_F8:
            return KEY_F8;

        case SCANCODE_F9:
            return KEY_F9;

        case SCANCODE_F10:
            return KEY_F10;

        case SCANCODE_F11:
            return KEY_F11;

        case SCANCODE_F12:
            return KEY_F12;
    }

    char normal = keyboard_map[key];
    char shifted = keyboard_shift_map[key];

    if (normal == 0)
    {
        return KEY_NONE;
    }

    if (normal >= 'a' && normal <= 'z')
    {
        if (keyboard_shift_pressed() ^ keyboard_caps_lock())
        {
            return shifted;
        }

        return normal;
    }

    if (keyboard_shift_pressed())
    {
        return shifted;
    }

    return normal;
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

uint16_t keyboard_getkey(void)
{
    while (1)
    {
        if (!keyboard_has_input())
        {
            continue;
        }

        uint8_t scancode = keyboard_read_scancode();

        if (scancode & SCANCODE_RELEASE)
        {
            keyboard_process_scancode(scancode);
            continue;
        }

        keyboard_process_scancode(scancode);

        uint16_t key = keyboard_scancode_to_key(scancode);

        if (key != KEY_NONE)
        {
            return key;
        }
    }
}