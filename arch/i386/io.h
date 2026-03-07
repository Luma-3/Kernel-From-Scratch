#ifndef __KFS_IO_H
#define __KFS_IO_H

#include <stdint.h>

/**
 * I/O port access functions.
 * These functions allow reading and writing to hardware ports.
 * The 'outb' function writes a byte to a specified port,
 * while the 'inb' function reads a byte from a specified port.
 */

/**
 * Writes a byte to the specified I/O port.
 *
 * @param port The I/O port to write to.
 * @param value The byte value to write to the port.
 */
void outb(uint16_t port, uint8_t value);

/**
 * Reads a byte from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The byte read from the port.
 */
uint8_t inb(uint16_t port);

#endif // !__KFS_IO_H
