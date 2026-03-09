
#include "keyevent.h"

#include <stdint.h>

void kdb_push_event(struct key_event event) {
    uint32_t next = (key_event_buffer_head + 1) % KEY_EVENT_BUFFER_SIZE;
    if (next != key_event_buffer_tail) {
        key_event_buffer[next] = event;
        key_event_buffer_head = next;
    }
}

struct key_event kdb_pop_event() {
    // Return an empty event if the buffer is empty
    struct key_event event = {0};
    if (key_event_buffer_head != key_event_buffer_tail) {
        event = key_event_buffer[key_event_buffer_tail];
        key_event_buffer_tail =
            (key_event_buffer_tail + 1) % KEY_EVENT_BUFFER_SIZE;
    }
    return event;
}

void kbd_handle_event(struct key_event event) {
    if (event.keycode == KEY_LEFTSHIFT || event.keycode == KEY_RIGHTSHIFT) {
        current_kbd_state.shift = event.state.pressed;
    }

    event.state.shift = current_kbd_state.shift;
    event.state.alt = current_kbd_state.alt;
    event.state.ctrl = current_kbd_state.ctrl;
    kdb_push_event(event);
}
