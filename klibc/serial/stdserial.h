#ifndef STDSERIAL_H
#define STDSERIAL_H

/**
 * Prints a character to the serial port.
 * @param *str the string to print
 */
void print_serial(const char *str);

/**
 * Prints a panic message to the serial port.
 * @param *str the string to print
 */
void panic_serial(const char *str);

#endif // !STDSERIAL_H
