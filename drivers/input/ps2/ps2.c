#include "ps2.h"
#include "i386/io.h"
#include "kdebug.h"
#include "keyevent.h"

#include <stdint.h>

#define PS2_TIMEOUT 100000

static int ps2_wait_input_empty()
{
	uint32_t timeout = PS2_TIMEOUT;
	while ((inb(PS2_STATUS_PORT) & 0x02) && --timeout)
		;
	return timeout > 0;
}

static int ps2_wait_output_full()
{
	uint32_t timeout = PS2_TIMEOUT;
	while (!(inb(PS2_STATUS_PORT) & 0x01) && --timeout)
		;
	return timeout > 0;
}

static void ps2_flush()
{
	uint32_t timeout = 1000;
	while ((inb(PS2_STATUS_PORT) & 0x01) && --timeout) {
		inb(PS2_DATA_PORT);
	}
}

void ps2_init()
{
	// Disable first PS/2 port
	if (!ps2_wait_input_empty()) return;
	outb(0xAD, PS2_STATUS_PORT);

	ps2_flush(); // Flush port to prevent remanent value

	// Get config byte
	if (!ps2_wait_input_empty()) return;
	outb(0x20, PS2_STATUS_PORT); // read config byte
	uint8_t config = ps2_read();
	if (config == 0) {
		return;
	}

	config &= ~0x01; // disable interrupt
	config &= ~0x10; // enable clock

	// Write config
	if (!ps2_wait_input_empty()) return;
	outb(0x60, PS2_STATUS_PORT);

	if (!ps2_wait_input_empty()) return;
	outb(config, PS2_DATA_PORT);

	// Enable keyboard
	if (!ps2_wait_input_empty()) return;
	outb(0xAE, PS2_STATUS_PORT);

	ps2_write(0xFF);

	(void)ps2_read(); // ACK (optionnel sur certaines machines)
	(void)ps2_read(); // Self-test OK (optionnel sur certaines machines)
}

uint8_t ps2_read()
{
	if (!ps2_wait_output_full()) {
		return 0;
	}
	return inb(PS2_DATA_PORT); // Read and return the data from the data port
}

void ps2_write(uint8_t data)
{
	if (!ps2_wait_input_empty()) {
		return;
	}
	outb(data, PS2_DATA_PORT);
}

uint8_t ps2_get_scanset()
{
	if (!ps2_wait_input_empty()) return 0xFF;
	outb(0xF0, PS2_DATA_PORT);

	if (!ps2_wait_output_full()) return 0xFF; // Wait ACK
	if (inb(PS2_DATA_PORT) != 0xFA) return 0xFF;

	if (!ps2_wait_input_empty()) return 0xFF;
	outb(0x00, PS2_DATA_PORT);

	if (!ps2_wait_output_full()) return 0xFF; // Wait ACK
	if (inb(PS2_DATA_PORT) != 0xFA) return 0xFF;

	if (!ps2_wait_output_full()) return 0xFF;
	return inb(PS2_DATA_PORT);
}

uint8_t ps2_set1_to_keycode(uint8_t scancode)
{
	uint8_t mask = 0x7F;
	return ps2_1_keycode_lookup_table[scancode & mask];
}

struct key_event create_key_event(uint8_t scancode)
{
	struct key_event event;
	event.keycode = ps2_set1_to_keycode(scancode);
	event.state.pressed =
		(scancode & 0x80) == 0; // Key is pressed if the highest bit is not set
	return event;
}

void ps2_keyboard_poll()
{
	if (!ps2_has_data()) return;
	uint8_t scancode = ps2_read();
	if (scancode == 0) return;
	struct key_event event = create_key_event(scancode);
	kbd_handle_event(event);
}

uint8_t ps2_has_data()
{
	return (inb(PS2_STATUS_PORT) & 0x01) != 0;
}

