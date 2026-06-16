
#include "keyevent.h"

#include <stdint.h>

static struct kbd_state current_kbd_state = {0, 0, 0, 0};

static struct key_event key_event_buffer[KEY_EVENT_BUFFER_SIZE];
static uint32_t key_event_buffer_head = 0;
static uint32_t key_event_buffer_tail = 0;

void kdb_push_event(struct key_event event) {
    uint32_t next = (key_event_buffer_head + 1) % KEY_EVENT_BUFFER_SIZE;

    if (next != key_event_buffer_tail) {
        key_event_buffer[key_event_buffer_head] = event;
        key_event_buffer_head = next;
    }
}

struct key_event kbd_pop_event() {
    // Return an empty event if the buffer is empty
    struct key_event event = {0};

    if (key_event_buffer_head != key_event_buffer_tail) {
        event = key_event_buffer[key_event_buffer_tail];
        key_event_buffer_tail =
            (key_event_buffer_tail + 1) % KEY_EVENT_BUFFER_SIZE;
    }
    return event;
}

static void update_kbd_state(struct key_event event) {
    if (event.keycode == KEY_LEFTSHIFT || event.keycode == KEY_RIGHTSHIFT) {
        current_kbd_state.shift = event.state.pressed;
    }
    if (event.keycode == KEY_LEFTALT || event.keycode == KEY_RIGHTALT) {
        current_kbd_state.alt = event.state.pressed;
    }
    if (event.keycode == KEY_LEFTCTRL) {
        current_kbd_state.ctrl = event.state.pressed;
    }
    if (event.keycode == KEY_CAPSLOCK && event.state.pressed) {
        current_kbd_state.caps_lock = !current_kbd_state.caps_lock;
    }
}

void kbd_handle_event(struct key_event event) {
    update_kbd_state(event);

    event.state.shift = current_kbd_state.shift;
    event.state.caps_lock = current_kbd_state.caps_lock;
    event.state.alt = current_kbd_state.alt;
    event.state.ctrl = current_kbd_state.ctrl;

    kdb_push_event(event);
}
