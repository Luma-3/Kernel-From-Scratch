#include "pit.h"
#include "io.h"

/**
 * @brief Performs a single hardware delay slice of up to 20,000 microseconds (20 ms).
 * Using slices of <= 20000 us ensures that (us * 193182) <= 3,863,640,000,
 * which fits comfortably within a 32-bit unsigned integer without requiring 64-bit division.
 */
static void pit_sleep_slice_us(uint32_t us) {
    if (us == 0) {
        return;
    }

    if (us > 20000) {
        us = 20000;
    }

    // Exact PIT tick calculation in pure 32-bit arithmetic:
    // PIT_FREQ / 1,000,000 = 1.193182 = 1 + 193182 / 1,000,000
    uint32_t ticks = us + (us * 193182U) / 1000000U;
    if (ticks == 0) {
        ticks = 1;
    }
    if (ticks > 0xFFFF) {
        ticks = 0xFFFF;
    }

    // 1. Disable Channel 2 gate and speaker data
    uint8_t port_b = inb(SYSTEM_PORT_B);
    outb((uint8_t)(port_b & 0xFC), SYSTEM_PORT_B);

    // 2. Set Channel 2 to Mode 0 (One-Shot), LSB then MSB, Binary
    // 0xB0 = 10 (Channel 2) 11 (LSB/MSB) 000 (Mode 0) 0 (Binary)
    outb(0xB0, PIT_PORT_COMMAND);

    // 3. Write divisor to Channel 2 (LSB then MSB)
    outb((uint8_t)(ticks & 0xFF), PIT_PORT_CHANNEL2);
    outb((uint8_t)((ticks >> 8) & 0xFF), PIT_PORT_CHANNEL2);

    // 4. Enable Channel 2 gate (bit 0 = 1) without speaker (bit 1 = 0)
    port_b = inb(SYSTEM_PORT_B);
    outb((uint8_t)((port_b & 0xFC) | 0x01), SYSTEM_PORT_B);

    // 5. Poll OUT2 (bit 5) until it becomes 1 (terminal count reached)
    while (!(inb(SYSTEM_PORT_B) & 0x20)) {
        __asm__ volatile("pause");
    }

    // 6. Disable Channel 2 gate
    port_b = inb(SYSTEM_PORT_B);
    outb((uint8_t)(port_b & 0xFC), SYSTEM_PORT_B);
}

void pit_sleep_us(uint32_t us) {
    while (us > 20000) {
        pit_sleep_slice_us(20000);
        us -= 20000;
    }
    if (us > 0) {
        pit_sleep_slice_us(us);
    }
}

void pit_sleep_ms(uint32_t ms) {
    while (ms > 20) {
        pit_sleep_slice_us(20000);
        ms -= 20;
    }
    if (ms > 0) {
        pit_sleep_slice_us(ms * 1000);
    }
}
