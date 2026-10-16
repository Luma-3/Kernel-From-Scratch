#ifndef SERIAL_H
#define SERIAL_H

#define PORT 0x3F8 // COM1 port address
#ifndef SERIAL_SUPPORT
# define SERIAL_SUPPORT 1
#endif

int init_serial();

int is_transmit_empty();

int serial_received();

char read_serial();

void write_serial(char a);

#endif // !SERIAL_H
