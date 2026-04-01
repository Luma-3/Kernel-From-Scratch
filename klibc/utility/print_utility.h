#ifndef __KFS_PRINT_UTILITY_H
#define __KFS_PRINT_UTILITY_H

int putchar(char c);
int putstr(const char *str);
int utoa(unsigned int value, char *str);
int putnbr(int value);
int putnbr_unsigned(unsigned int value);
int lutohex(unsigned long value, char *str, int uppercase);
int puthex(unsigned long value, int uppercase);
int putpointer(void *ptr);

#endif //__KFS_PRINT_UTILITY_H