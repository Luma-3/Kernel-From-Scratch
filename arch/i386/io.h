#ifndef KFS_IO_H
#define KFS_IO_H

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
 * @param value The byte value to write to the port.
 * @param port The I/O port to write to.
 */
void outb(uint8_t value, uint16_t port);

/**
 * Reads a byte from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The byte read from the port.
 */
uint8_t inb(uint16_t port);

/**
 * Writes a word (16-bit) to the specified I/O port.
 *
 * @param value The word value to write to the port.
 * @param port The I/O port to write to.
 */
void outw(uint16_t value, uint16_t port);

/**
 * Reads a word (16-bit) from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The word read from the port.
 */
uint16_t inw(uint16_t port);

/**
 * Writes a double word (32-bit) to the specified I/O port.
 *
 * @param value The double word value to write to the port.
 * @param port The I/O port to write to.
 */
void outl(uint32_t value, uint16_t port);

/**
 * Reads a double word (32-bit) from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The double word read from the port.
 */
uint32_t inl(uint16_t port);

#endif // KFS_IO_H
