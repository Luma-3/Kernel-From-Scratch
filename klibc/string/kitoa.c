
int kitoa(int value, char *str) {
    if(!str) {
        return 0;
    }
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    if (value < 0) {
        value = -value;
        *ptr++ = '-';
        ptr1++;
    }

    do {
        tmp_value = value;
        value /= 10;
        *ptr++ = '0' + (tmp_value - value * 10);
    } while (value);

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return ptr - str + 1;
}