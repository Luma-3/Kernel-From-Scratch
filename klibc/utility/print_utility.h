#ifndef KFS_PRINT_UTILITY_H
#define KFS_PRINT_UTILITY_H

int putchar(char c);
int putstr(const char *str);
int utoa(unsigned int value, char *str);
int putnbr(int value);
int putnbr_unsigned(unsigned int value);
int lutoa(unsigned long value, char *str);
int litoa(long value, char *str);
int llutoa(unsigned long long value, char *str);
int llitoa(long long value, char *str);
int putnbr_long(long value);
int putnbr_unsigned_long(unsigned long value);
int putnbr_unsigned_long_long(unsigned long long value);
int putnbr_long_long(long long value);
int lutohex(unsigned long value, char *str, int uppercase);
int llutoa(unsigned long long value, char *str);
int puthex_long_long(unsigned long long value, int uppercase);
int puthex(unsigned long value, int uppercase);
int putpointer(void *ptr);

#endif // KFS_PRINT_UTILITY_H
