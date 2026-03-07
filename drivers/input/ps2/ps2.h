#ifndef __KFS_PS2_H
#define __KFS_PS2_H

#include <stdint.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

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

#endif // __KFS_PS2_H
