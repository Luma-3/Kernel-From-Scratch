#include <stdint.h>
void outb(uint8_t value, uint16_t port) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void cli() { __asm__ volatile("cli"); }

void pit_init(uint32_t freq_hz) {
  uint32_t divisor = 1193182 / freq_hz;
  outb(0x34, 0x43); // Command byte: channel 0, access mode lobyte/hibyte, mode
                    // 2 (rate generator)
  outb(divisor & 0xFF, 0x40);        // Send low byte of divisor
  outb((divisor >> 8) & 0xFF, 0x40); // Send high byte of divisor
}

uint16_t pit_read() {
  cli();
  outb(0x00, 0x43);         // Latch current count value
  uint8_t low = inb(0x40);  // Read low byte
  uint8_t high = inb(0x40); // Read high byte
  return low | (high << 8); // Combine into a single 16-bit value
}

void ps2_init() {
  outb(0xAE, 0x64); // Enable PS/2 port via controller command
  outb(0x20, 0x64); // Read controller configuration byte
  uint8_t config = inb(0x60);
  config &= ~0x10;    // Enable keyboard interrupt (clear bit 4)
  outb(0x60, 0x64);   // Write modified configuration byte back
  outb(config, 0x60); // pass the modified config byte to the data port
}
