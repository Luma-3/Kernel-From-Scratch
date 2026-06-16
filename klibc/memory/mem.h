//
// Created by gcaptari on 2/27/26.
//

#ifndef __KFS_MEM_H
#define __KFS_MEM_H

#include <stddef.h>
#include <stdint.h>

/** Sets a block of memory to a specified value.
 * @param s Pointer to the block of memory to fill.
 * @param c The value to set (passed as an int, but interpreted as an unsigned char).
 * @param n The number of bytes to set.
 * @return A pointer to the block of memory filled with the specified value.
 */
void *kmemset(void *s, int c, size_t n);

/**
 * Moves count bytes from memory area src to memory area dest. The memory areas may overlap: copying takes place as though the bytes in src are first copied into a temporary array that does not overlap src or dest, and the bytes are then copied from the temporary array to dest.
 * @param dest Pointer to the destination memory area.
 * @param src Pointer to the source memory area.
 * @param count Number of bytes to move.
 * @return A pointer to the destination memory area dest.
 */
void *kmemmove(void *dest, const void *src, size_t count);

/**
 * Copies count bytes from memory area src to memory area dest. The memory areas must not overlap. Use kmemmove if the memory areas do overlap.
 * @param dest Pointer to the destination memory area.
 * @param src Pointer to the source memory area.
 * @param count Number of bytes to copy.
 * @return A pointer to the destination memory area dest.
 */
void *kmemcpy(void *dest, const void *src, size_t count);

#endif //__KFS_MEM_H