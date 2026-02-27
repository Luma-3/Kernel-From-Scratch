#ifndef __KFS_PS2_H
#define __KFS_PS2_H

#include <stdint.h>

void outb(uint8_t value, uint16_t port);
uint8_t inb(uint16_t port);

void pit_init(uint32_t freq_hz);
void pit_wait(uint32_t ticks);
void ps2_init();

uint16_t pit_read();
#endif // __KFS_PS2_H
