#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <stdint.h>

__attribute__((always_inline)) static inline uint32_t get_ebp(void)
{
	uint32_t ebp;
	__asm__ volatile("mov %%ebp, %0" : "=r"(ebp));
	return ebp;
}

#endif // !STACK_TRACE_H
