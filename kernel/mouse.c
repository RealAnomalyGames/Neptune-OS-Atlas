#include "mouse.h"
#include "io.h"
#include "terminal.h"

#define MOUSE_DATA_PORT       0x60
#define MOUSE_STATUS_PORT     0x64
#define MOUSE_COMMAND_PORT    0x64

#define PS2_STATUS_OUTPUT_FULL 0x01
#define PS2_STATUS_INPUT_FULL  0x02

#define PS2_COMMAND_SEND_MOUSE 0xD4

#define MOUSE_PACKET_SIZE 3

#define MOUSE_PACKET_LEFT_BUTTON   0x01
#define MOUSE_PACKET_RIGHT_BUTTON  0x02
#define MOUSE_PACKET_MIDDLE_BUTTON 0x04

#define MOUSE_PACKET_X_SIGN        0x10
#define MOUSE_PACKET_Y_SIGN        0x20
#define MOUSE_PACKET_X_OVERFLOW    0x40
#define MOUSE_PACKET_Y_OVERFLOW    0x80

#define PS2_COMMAND_ENABLE_AUX      0xA8
#define PS2_COMMAND_READ_CONFIG     0x20
#define PS2_COMMAND_WRITE_CONFIG    0x60

#define PS2_CONFIG_IRQ12             0x02
#define PS2_CONFIG_MOUSE_CLOCK       0x20

#define MOUSE_COMMAND_DISABLE        0xF5
#define MOUSE_COMMAND_ENABLE         0xF4

#define MOUSE_ACK                    0xFA

static MouseState mouse_state;
static CursorState cursor_state;

static uint8_t mouse_packet[MOUSE_PACKET_SIZE];
static uint8_t mouse_packet_index;

static uint8_t mouse_read_byte(void);

static void mouse_write_controller_config(
    uint8_t config
);

static uint8_t mouse_read_status(void)
{
    return inb(MOUSE_STATUS_PORT);
}

static void mouse_wait_input(void)
{
    uint32_t timeout = 100000;

    while ((mouse_read_status() & PS2_STATUS_INPUT_FULL) != 0)
    {
        if (timeout == 0)
        {
            return;
        }

        timeout--;
    }
}

static void mouse_wait_output(void)
{
    uint32_t timeout = 100000;

    while ((mouse_read_status() & PS2_STATUS_OUTPUT_FULL) == 0)
    {
        if (timeout == 0)
        {
            return;
        }

        timeout--;
    }
}

static void mouse_controller_command(uint8_t command)
{
    mouse_wait_input();
    outb(MOUSE_COMMAND_PORT, command);
}

static uint8_t mouse_read_controller_config(void)
{
    mouse_controller_command(
        PS2_COMMAND_READ_CONFIG
    );

    return mouse_read_byte();
}

static void mouse_send_command(uint8_t command)
{
    mouse_controller_command(PS2_COMMAND_SEND_MOUSE);

    mouse_wait_input();
    outb(MOUSE_DATA_PORT, command);
}

static uint8_t mouse_read_byte(void)
{
    mouse_wait_output();
    return inb(MOUSE_DATA_PORT);
}

static int mouse_wait_for_ack(void)
{
    uint8_t response;

    response = mouse_read_byte();

    if (response == MOUSE_ACK)
    {
        return 1;
    }

    return 0;
}

static void mouse_decode_packet(uint8_t packet[3])
{
    int32_t delta_x;
    int32_t delta_y;

    if ((packet[0] & MOUSE_PACKET_X_OVERFLOW) != 0)
    {
        delta_x = 0;
    }
    else
    {
        delta_x = (int8_t)packet[1];
    }

    if ((packet[0] & MOUSE_PACKET_Y_OVERFLOW) != 0)
    {
        delta_y = 0;
    }
    else
    {
        delta_y = (int8_t)packet[2];
    }

    mouse_state.delta_x = delta_x;
    mouse_state.delta_y = delta_y;

    mouse_state.buttons = packet[0] &
        (MOUSE_PACKET_LEFT_BUTTON |
         MOUSE_PACKET_RIGHT_BUTTON |
         MOUSE_PACKET_MIDDLE_BUTTON);

    mouse_state.left_button =
        (mouse_state.buttons & MOUSE_BUTTON_LEFT) != 0;

    mouse_state.right_button =
        (mouse_state.buttons & MOUSE_BUTTON_RIGHT) != 0;

    mouse_state.middle_button =
        (mouse_state.buttons & MOUSE_BUTTON_MIDDLE) != 0;
}

static void mouse_process_packet(uint8_t packet[3])
{
    mouse_decode_packet(packet);
    cursor_update();
}

static void mouse_receive_byte(uint8_t value)
{
    /*
     * The first byte must have bit 3 set.
     * This prevents us from starting a packet
     * in the middle of a sequence.
     */
    if (mouse_packet_index == 0)
    {
        if ((value & 0x08) == 0)
        {
            return;
        }
    }

    mouse_packet[mouse_packet_index] = value;
    mouse_packet_index++;

    if (mouse_packet_index >= MOUSE_PACKET_SIZE)
    {
        mouse_process_packet(mouse_packet);
        mouse_packet_index = 0;
    }
}

void mouse_initialize(void)
{
    uint8_t config;

    /*
     * Initialize software mouse state.
     */
    mouse_state.x = MOUSE_MAX_X / 2;
    mouse_state.y = MOUSE_MAX_Y / 2;
    mouse_state.delta_x = 0;
    mouse_state.delta_y = 0;
    mouse_state.buttons = 0;
    mouse_state.left_button = 0;
    mouse_state.right_button = 0;
    mouse_state.middle_button = 0;

    mouse_packet[0] = 0;
    mouse_packet[1] = 0;
    mouse_packet[2] = 0;
    mouse_packet_index = 0;

    /*
     * Enable the PS/2 auxiliary device.
     *
     * The auxiliary device is the mouse.
     */
    mouse_controller_command(
        PS2_COMMAND_ENABLE_AUX
    );

    /*
     * Read the current PS/2 controller
     * configuration byte.
     */
    config = mouse_read_controller_config();

    /*
     * Enable IRQ12.
     */
    config |= PS2_CONFIG_IRQ12;

    /*
     * Enable the mouse clock.
     */
    config &= (uint8_t)~PS2_CONFIG_MOUSE_CLOCK;

    /*
     * Write the updated configuration.
     */
    mouse_write_controller_config(config);

    /*
     * Disable mouse data reporting while
     * initialization is being completed.
     */
    mouse_send_command(MOUSE_COMMAND_DISABLE);

    /*
     * Read and consume the mouse ACK.
     */
    mouse_wait_for_ack();

    /*
     * Enable mouse data reporting.
     */
    mouse_send_command(MOUSE_COMMAND_ENABLE);

    /*
     * Read and consume the mouse ACK.
     */
    mouse_wait_for_ack();
}

void mouse_get_state(MouseState* state)
{
    if (state == 0)
    {
        return;
    }

    *state = mouse_state;
}

int32_t mouse_get_x(void)
{
    return mouse_state.x;
}

int32_t mouse_get_y(void)
{
    return mouse_state.y;
}

int32_t mouse_get_delta_x(void)
{
    return mouse_state.delta_x;
}

int32_t mouse_get_delta_y(void)
{
    return mouse_state.delta_y;
}

uint8_t mouse_get_buttons(void)
{
    return mouse_state.buttons;
}

uint8_t mouse_is_left_button_pressed(void)
{
    return mouse_state.left_button;
}

uint8_t mouse_is_right_button_pressed(void)
{
    return mouse_state.right_button;
}

uint8_t mouse_is_middle_button_pressed(void)
{
    return mouse_state.middle_button;
}

void mouse_interrupt_handler(void)
{
    uint8_t value;

    value = inb(MOUSE_DATA_PORT);

    mouse_receive_byte(value);
}

void cursor_initialize(void)
{
    cursor_state.x = MOUSE_MAX_X / 2;
    cursor_state.y = MOUSE_MAX_Y / 2;
    cursor_state.visible = 1;
}

void cursor_update(void)
{
    cursor_state.x += mouse_state.delta_x;
    cursor_state.y -= mouse_state.delta_y;

    if (cursor_state.x < MOUSE_MIN_X)
    {
        cursor_state.x = MOUSE_MIN_X;
    }

    if (cursor_state.x > MOUSE_MAX_X)
    {
        cursor_state.x = MOUSE_MAX_X;
    }

    if (cursor_state.y < MOUSE_MIN_Y)
    {
        cursor_state.y = MOUSE_MIN_Y;
    }

    if (cursor_state.y > MOUSE_MAX_Y)
    {
        cursor_state.y = MOUSE_MAX_Y;
    }
}

void cursor_get_state(CursorState* state)
{
    if (state == 0)
    {
        return;
    }

    *state = cursor_state;
}

int32_t cursor_get_x(void)
{
    return cursor_state.x;
}

int32_t cursor_get_y(void)
{
    return cursor_state.y;
}

void cursor_set_visible(uint8_t visible)
{
    cursor_state.visible = visible != 0;
}

uint8_t cursor_is_visible(void)
{
    return cursor_state.visible;
}

void mouse_clear_delta(void)
{
    mouse_state.delta_x = 0;
    mouse_state.delta_y = 0;
}

void mouse_display_status(void)
{
    CursorState cursor;
    uint8_t buttons;

    cursor_get_state(&cursor);

    buttons = mouse_get_buttons();

    terminal_write_at(
        "Mouse X: ",
        TERMINAL_HEIGHT - 1,
        0
    );

    terminal_write_uint_at(
        (uint32_t)cursor.x,
        TERMINAL_HEIGHT - 1,
        9
    );

    terminal_write_at(
        " Y: ",
        TERMINAL_HEIGHT - 1,
        13
    );

    terminal_write_uint_at(
        (uint32_t)cursor.y,
        TERMINAL_HEIGHT - 1,
        17
    );

    terminal_write_at(
        " | L: ",
        TERMINAL_HEIGHT - 1,
        21
    );

    terminal_write_uint_at(
        (uint32_t)(
            (buttons & MOUSE_BUTTON_LEFT) != 0
        ),
        TERMINAL_HEIGHT - 1,
        27
    );

    terminal_write_at(
        " R: ",
        TERMINAL_HEIGHT - 1,
        28
    );

    terminal_write_uint_at(
        (uint32_t)(
            (buttons & MOUSE_BUTTON_RIGHT) != 0
        ),
        TERMINAL_HEIGHT - 1,
        32
    );

    terminal_write_at(
        " M: ",
        TERMINAL_HEIGHT - 1,
        33
    );

    terminal_write_uint_at(
        (uint32_t)(
            (buttons & MOUSE_BUTTON_MIDDLE) != 0
        ),
        TERMINAL_HEIGHT - 1,
        37
    );
}

static void mouse_write_controller_config(
    uint8_t config
)
{
    mouse_controller_command(
        PS2_COMMAND_WRITE_CONFIG
    );

    mouse_wait_input();
    outb(MOUSE_DATA_PORT, config);
}