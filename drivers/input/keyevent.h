#ifndef __KFS_KEYEVENT_H
#define __KFS_KEYEVENT_H

/**
 * Keycode definitions for the KFS operating system.
 * These keycodes are based on the standard PS/2 keyboard scancodes.
 *
 * for the moment, the translation of controller 8042 isn't disabled, this means
 * keycodes converter is based on Set 1 scancodes, which are the most common
 * ones used by PS/2 keyboards.
 */

#include <stdint.h>
#define KEY_1 0x02
#define KEY_2 0x03
#define KEY_3 0x04
#define KEY_4 0x05
#define KEY_5 0x06
#define KEY_6 0x07
#define KEY_7 0x08
#define KEY_8 0x09
#define KEY_9 0x0A
#define KEY_0 0x0B

#define KEY_ESC 0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_ENTER 0x1C
#define KEY_CAPSLOCK 0x3A

#define KEY_Q 0x10
#define KEY_W 0x11
#define KEY_E 0x12
#define KEY_R 0x13
#define KEY_T 0x14
#define KEY_Y 0x15
#define KEY_U 0x16
#define KEY_I 0x17
#define KEY_O 0x18
#define KEY_P 0x19
#define KEY_A 0x1E
#define KEY_S 0x1F
#define KEY_D 0x20
#define KEY_F 0x21
#define KEY_G 0x22
#define KEY_H 0x23
#define KEY_J 0x24
#define KEY_K 0x25
#define KEY_L 0x26
#define KEY_Z 0x2C
#define KEY_X 0x2D
#define KEY_C 0x2E
#define KEY_V 0x2F
#define KEY_B 0x30
#define KEY_N 0x31
#define KEY_M 0x32

#define KEY_SPACE 0x39

#define KEY_EQUALS 0x0D
#define KEY_MINUS 0x0C

#define KEY_BACKSLASH 0x2B
#define KEY_SLASH 0x35

#define KEY_APOSTROPHE 0x28
#define KEY_GRAVE 0x29
#define KEY_COMMA 0x33
#define KEY_DOT 0x34
#define KEY_SEMICOLON 0x27

#define KEY_RIGHTBRACKET 0x1B
#define KEY_LEFTBRACKET 0x1A

#define KEY_LEFTCTRL 0x1D
#define KEY_RIGHTCTRL 0x9D

#define KEY_RIGHTSHIFT 0x36
#define KEY_LEFTSHIFT 0x2A

struct key_event {
    uint8_t keycode;
    struct {
        uint8_t pressed : 1;
        uint8_t shift : 1;
        uint8_t ctrl : 1;
        uint8_t alt : 1;
    } state;
};

struct kbd_state {
    uint8_t shift : 1;
    uint8_t ctrl : 1;
    uint8_t alt : 1;
};

static struct kbd_state current_kbd_state = {0};

#define KEY_EVENT_BUFFER_SIZE 128

static struct key_event key_event_buffer[KEY_EVENT_BUFFER_SIZE];
static uint32_t key_event_buffer_head = 0;
static uint32_t key_event_buffer_tail = 0;

/**
 * Handle a new simple key event, to manage shift, caps lock, alt, etc..
 *
 * @note Auto push Key event
 */
void kbd_handle_event(struct key_event event);

/**
 * Pushes a key event into the buffer. If the buffer is full, the event will be
 * discarded.
 *
 * @param event The key event to push into the buffer.
 */
void kdb_push_event(struct key_event event);

/**
 * Pops a key event from the buffer. If the buffer is empty, an empty event will
 * be returned.
 *
 * @return The key event popped from the buffer.
 */
struct key_event kdb_pop_event();

#endif // !__KFS_KEYEVENT_H
