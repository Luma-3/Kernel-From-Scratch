#include "ps2.h"
#include "io.h"
#include "keyevent.h"

#include <stdint.h>

void cli() { __asm__ volatile("cli"); }

void ps2_init() {
    outb(0xAE, PS2_STATUS_PORT); // Enable PS/2 port via controller command
    outb(0x20, PS2_STATUS_PORT); // Read controller configuration byte
    uint8_t config = inb(PS2_DATA_PORT);
    config &= ~0x10;             // Enable keyboard interrupt (clear bit 4)
    outb(0x60, PS2_STATUS_PORT); // Write modified configuration byte back
    outb(config,
         PS2_DATA_PORT); // pass the modified config byte to the data port
}

uint8_t ps2_read() {
    while ((inb(PS2_STATUS_PORT) & 0x01) == 0)
        ;                      // Wait until data is available
    return inb(PS2_DATA_PORT); // Read and return the data from the data port
}

uint8_t ps2_set1_to_keycode(uint8_t scancode) {
    uint8_t mask = 0x7F;
    return ps2_1_keycode_lookup_table[scancode & mask];
}

struct key_event create_key_event(uint8_t scancode) {
    struct key_event event;
    event.keycode = ps2_set1_to_keycode(scancode);
    event.state.pressed =
        (scancode & 0x80) == 0; // Key is pressed if the highest bit is not set
    return event;
}

void ps2_keyboard_poll() {
    uint8_t scancode = ps2_read();
    struct key_event event = create_key_event(scancode);
    kbd_handle_event(event);
}
