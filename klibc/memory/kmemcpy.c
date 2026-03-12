#include <stddef.h>

void *kmemcpy(void *dest, const void *src, size_t count) {
	char *d = (char *) dest;
	const char *s = (const char *) src;

	while (count--) {
		*d++ = *s++;
	}

	return dest;
}