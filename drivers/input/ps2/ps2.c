#include "ps2.h"
#include "io.h"
#include "keyevent.h"
#include "printk.h"

#include <stdint.h>

void cli() { __asm__ volatile("cli"); }

static void ps2_wait_input_empty() {
    while (inb(PS2_STATUS_PORT) & 0x02)
        ;
}

static void ps2_wait_output_full() {
    while (!(inb(PS2_STATUS_PORT) & 0x01))
        ;
}

static void ps2_flush() {
    while (inb(PS2_STATUS_PORT) & 0x01) {
        inb(PS2_DATA_PORT);
    }
}

void ps2_init() {
    // Disable first PS/2 port
    ps2_wait_input_empty();
    outb(0xAD, PS2_STATUS_PORT);

    ps2_flush(); // Flush port to prevent remanent value

    // Get config byte
    ps2_wait_input_empty();
    outb(0x20, PS2_STATUS_PORT); // read config byte
    uint8_t config = ps2_read();

    config &= ~0x01; // disable interrupt
    config &= ~0x10; // enable clock

    // Write config
    ps2_wait_input_empty();
    outb(0x60, PS2_STATUS_PORT);

    ps2_wait_input_empty();
    outb(config, PS2_DATA_PORT);

    // Enable keyboard
    ps2_wait_input_empty();
    outb(0xAE, PS2_STATUS_PORT);

    ps2_write(0xFF);

    if (ps2_read() != 0xFA)
        return; // ACK
    if (ps2_read() != 0xAA)
        return; // Slef-test OK
}

uint8_t ps2_read() {
    ps2_wait_output_full();
    return inb(PS2_DATA_PORT); // Read and return the data from the data port
}

void ps2_write(uint8_t data) {
    ps2_wait_input_empty();
    outb(data, PS2_DATA_PORT);
}

uint8_t ps2_get_scanset() {
    ps2_wait_input_empty();
    outb(0xF0, PS2_DATA_PORT);

    ps2_wait_output_full(); // Wait ACK
    if (inb(PS2_DATA_PORT) != 0xFA)
        return 0xFF;

    ps2_wait_input_empty();
    outb(0x00, PS2_DATA_PORT);

    ps2_wait_output_full(); // Wait ACK
    if (inb(PS2_DATA_PORT) != 0xFA)
        return 0xFF;

    ps2_wait_output_full();
    return inb(PS2_DATA_PORT);
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

    // printk(KERNEL_LOG_LEVEL_DEBUG, "Scancode: %x -> Keycode: %x, State:
    // %s\n",
    //        scancode, event.keycode,
    //        event.state.pressed ? "Pressed" : "Released");

}
