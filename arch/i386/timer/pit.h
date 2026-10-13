#ifndef KFS_PIT_H
#define KFS_PIT_H

#include <stdint.h>

/**
 * PIT (Programmable Interval Timer 8254) definitions.
 * Base frequency of the PIT oscillator: 1193182 Hz.
 */
#define PIT_BASE_FREQUENCY 1193182ULL
#define PIT_PORT_CHANNEL0  0x40
#define PIT_PORT_CHANNEL1  0x41
#define PIT_PORT_CHANNEL2  0x42
#define PIT_PORT_COMMAND   0x43
#define SYSTEM_PORT_B      0x61

/**
 * @brief Busy-wait for a specified number of microseconds using PIT Channel 2.
 * The 16-bit counter can hold up to 65535 ticks (~54924 us).
 * Higher values are clamped to 54000 us.
 * 
 * @param us Duration in microseconds.
 */
void pit_sleep_us(uint32_t us);

/**
 * @brief Busy-wait for a specified number of milliseconds using PIT Channel 2.
 * Long durations are safely segmented into chunks of <= 50 ms.
 * 
 * @param ms Duration in milliseconds.
 */
void pit_sleep_ms(uint32_t ms);

#endif // KFS_PIT_H
