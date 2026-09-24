#ifndef KDEBUG_H
#define KDEBUG_H

#include "printk.h"

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

/**
 * Prints a backtrace of the current stack frames.
 * @param max_frames the maximum number of frames to print
 */
void backtrace(int max_frames);

#endif // !KDEBUG_H
