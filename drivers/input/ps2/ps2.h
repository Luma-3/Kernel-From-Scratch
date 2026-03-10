#ifndef __KFS_PS2_H
#define __KFS_PS2_H

#include "keyevent.h"

#include <stdint.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

///******************** PS/2 Scancode Set 1 ********************///

#define PS2_1SCANCODE_ESC 0x01
#define PS2_1SCANCODE_1 0x02
#define PS2_1SCANCODE_2 0x03
#define PS2_1SCANCODE_3 0x04
#define PS2_1SCANCODE_4 0x05
#define PS2_1SCANCODE_5 0x06
#define PS2_1SCANCODE_6 0x07
#define PS2_1SCANCODE_7 0x08
#define PS2_1SCANCODE_8 0x09
#define PS2_1SCANCODE_9 0x0A
#define PS2_1SCANCODE_0 0x0B
#define PS2_1SCANCODE_MINUS 0x0C
#define PS2_1SCANCODE_EQUALS 0x0D
#define PS2_1SCANCODE_BACKSPACE 0x0E
#define PS2_1SCANCODE_TAB 0x0F
#define PS2_1SCANCODE_Q 0x10
#define PS2_1SCANCODE_W 0x11
#define PS2_1SCANCODE_E 0x12
#define PS2_1SCANCODE_R 0x13
#define PS2_1SCANCODE_T 0x14
#define PS2_1SCANCODE_Y 0x15
#define PS2_1SCANCODE_U 0x16
#define PS2_1SCANCODE_I 0x17
#define PS2_1SCANCODE_O 0x18
#define PS2_1SCANCODE_P 0x19
#define PS2_1SCANCODE_LEFTBRACKET 0x1A
#define PS2_1SCANCODE_RIGHTBRACKET 0x1B
#define PS2_1SCANCODE_ENTER 0x1C
#define PS2_1SCANCODE_LEFTCTRL 0x1D
#define PS2_1SCANCODE_A 0x1E
#define PS2_1SCANCODE_S 0x1F
#define PS2_1SCANCODE_D 0x20
#define PS2_1SCANCODE_F 0x21
#define PS2_1SCANCODE_G 0x22
#define PS2_1SCANCODE_H 0x23
#define PS2_1SCANCODE_J 0x24
#define PS2_1SCANCODE_K 0x25
#define PS2_1SCANCODE_L 0x26
#define PS2_1SCANCODE_SEMICOLON 0x27
#define PS2_1SCANCODE_APOSTROPHE 0x28
#define PS2_1SCANCODE_GRAVE 0x29
#define PS2_1SCANCODE_LEFTSHIFT 0x2A
#define PS2_1SCANCODE_BACKSLASH 0x2B
#define PS2_1SCANCODE_Z 0x2C
#define PS2_1SCANCODE_X 0x2D
#define PS2_1SCANCODE_C 0x2E
#define PS2_1SCANCODE_V 0x2F
#define PS2_1SCANCODE_B 0x30
#define PS2_1SCANCODE_N 0x31
#define PS2_1SCANCODE_M 0x32
#define PS2_1SCANCODE_COMMA 0x33
#define PS2_1SCANCODE_DOT 0x34
#define PS2_1SCANCODE_SLASH 0x35
#define PS2_1SCANCODE_RIGHTSHIFT 0x36
#define PS2_1SCANCODE_KP_MULTIPLY 0x37
#define PS2_1SCANCODE_LEFTALT 0x38
#define PS2_1SCANCODE_SPACE 0x39
#define PS2_1SCANCODE_CAPSLOCK 0x3A
#define PS2_1SCANCODE_F1 0x3B
#define PS2_1SCANCODE_F2 0x3C
#define PS2_1SCANCODE_F3 0x3D
#define PS2_1SCANCODE_F4 0x3E
#define PS2_1SCANCODE_F5 0x3F
#define PS2_1SCANCODE_F6 0x40
#define PS2_1SCANCODE_F7 0x41
#define PS2_1SCANCODE_F8 0x42
#define PS2_1SCANCODE_F9 0x43
#define PS2_1SCANCODE_F10 0x44
#define PS2_1SCANCODE_KP_NUMLOCK 0x45
#define PS2_1SCANCODE_SCROLLLOCK 0x46
#define PS2_1SCANCODE_KP_7 0x47
#define PS2_1SCANCODE_KP_8 0x48
#define PS2_1SCANCODE_KP_9 0x49
#define PS2_1SCANCODE_KP_SUBTRACT 0x4A
#define PS2_1SCANCODE_KP_4 0x4B
#define PS2_1SCANCODE_KP_5 0x4C
#define PS2_1SCANCODE_KP_6 0x4D
#define PS2_1SCANCODE_KP_ADD 0x4E
#define PS2_1SCANCODE_KP_1 0x4F
#define PS2_1SCANCODE_KP_2 0x50
#define PS2_1SCANCODE_KP_3 0x51
#define PS2_1SCANCODE_KP_0 0x52
#define PS2_1SCANCODE_KP_DOT 0x53
#define PS2_1SCANCODE_F11 0x57
#define PS2_1SCANCODE_F12 0x58

///*************** PS/2 Lookup Tables Keycode Mapping ****************///

static const uint8_t ps2_1_keycode_lookup_table[256] = {
    [PS2_1SCANCODE_ESC] = KEY_ESC,
    [PS2_1SCANCODE_1] = KEY_1,
    [PS2_1SCANCODE_2] = KEY_2,
    [PS2_1SCANCODE_3] = KEY_3,
    [PS2_1SCANCODE_4] = KEY_4,
    [PS2_1SCANCODE_5] = KEY_5,
    [PS2_1SCANCODE_6] = KEY_6,
    [PS2_1SCANCODE_7] = KEY_7,
    [PS2_1SCANCODE_8] = KEY_8,
    [PS2_1SCANCODE_9] = KEY_9,
    [PS2_1SCANCODE_0] = KEY_0,
    [PS2_1SCANCODE_MINUS] = KEY_MINUS,
    [PS2_1SCANCODE_EQUALS] = KEY_EQUALS,
    [PS2_1SCANCODE_BACKSPACE] = KEY_BACKSPACE,
    [PS2_1SCANCODE_TAB] = KEY_TAB,
    [PS2_1SCANCODE_Q] = KEY_Q,
    [PS2_1SCANCODE_W] = KEY_W,
    [PS2_1SCANCODE_E] = KEY_E,
    [PS2_1SCANCODE_R] = KEY_R,
    [PS2_1SCANCODE_T] = KEY_T,
    [PS2_1SCANCODE_Y] = KEY_Y,
    [PS2_1SCANCODE_U] = KEY_U,
    [PS2_1SCANCODE_I] = KEY_I,
    [PS2_1SCANCODE_O] = KEY_O,
    [PS2_1SCANCODE_P] = KEY_P,
    [PS2_1SCANCODE_LEFTBRACKET] = KEY_LEFTBRACKET,
    [PS2_1SCANCODE_RIGHTBRACKET] = KEY_RIGHTBRACKET,
    [PS2_1SCANCODE_ENTER] = KEY_ENTER,
    [PS2_1SCANCODE_LEFTCTRL] = KEY_LEFTCTRL,
    [PS2_1SCANCODE_A] = KEY_A,
    [PS2_1SCANCODE_S] = KEY_S,
    [PS2_1SCANCODE_D] = KEY_D,
    [PS2_1SCANCODE_F] = KEY_F,
    [PS2_1SCANCODE_G] = KEY_G,
    [PS2_1SCANCODE_H] = KEY_H,
    [PS2_1SCANCODE_J] = KEY_J,
    [PS2_1SCANCODE_K] = KEY_K,
    [PS2_1SCANCODE_L] = KEY_L,
    [PS2_1SCANCODE_SEMICOLON] = KEY_SEMICOLON,
    [PS2_1SCANCODE_APOSTROPHE] = KEY_APOSTROPHE,
    [PS2_1SCANCODE_GRAVE] = KEY_GRAVE,
    [PS2_1SCANCODE_LEFTSHIFT] = KEY_LEFTSHIFT,
    [PS2_1SCANCODE_RIGHTSHIFT] = KEY_RIGHTSHIFT,
    [PS2_1SCANCODE_BACKSLASH] = KEY_BACKSLASH,
    [PS2_1SCANCODE_Z] = KEY_Z,
    [PS2_1SCANCODE_X] = KEY_X,
    [PS2_1SCANCODE_C] = KEY_C,
    [PS2_1SCANCODE_V] = KEY_V,
    [PS2_1SCANCODE_B] = KEY_B,
    [PS2_1SCANCODE_N] = KEY_N,
    [PS2_1SCANCODE_M] = KEY_M,

    [PS2_1SCANCODE_SPACE] = KEY_SPACE,
    [PS2_1SCANCODE_CAPSLOCK] = KEY_CAPSLOCK,

};
///******************** PS/2 Controller Interface ********************///

/**
 * This header file defines the interface for interacting with the PS/2
 * controller,
 */

/**
 * Initializes the PS/2 controller and enables keyboard interrupts.
 */
void ps2_init();

/**
 * Reads a byte of data from the PS/2 data port.
 * This function blocks until data is available.
 *
 * @return The byte read from the PS/2 data port.
 */
uint8_t ps2_read();

/**
 * Converts a PS/2 Set 1 scancode to a keycode defined in keyevent.h.
 *
 * @param scancode The PS/2 Set 1 scancode to convert.
 * @return The corresponding keycode defined in keyevent.h, or 0 if the
 *         scancode is not recognized.
 */
uint8_t ps2_set1_to_keycode(uint8_t scancode);

/**
 * Polls the PS/2 keyboard for key events. This function should be called
 * repeatedly in the main loop to process keyboard input.
 * It reads scancodes from the PS/2 data port, converts them to key events,
 * and pushes them into the key event buffer.
 */
void ps2_keyboard_poll();

#endif // __KFS_PS2_H
