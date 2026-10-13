#include <stdint.h>
#include "i386/stack/stack_trace.h"
#include "printf.h"
#include "trace/ksyms.h"

static void format_print(uint32_t index, uint32_t addr, const char *symbol,
						 uint32_t offset)
{
	printf(" #%d - 0x%x: %s\t+0x%x\n", index, addr, symbol, offset);
}

static void print_header(uint32_t max_frames)
{
	printf("Backtrace (max %d frames):\n", max_frames);
}

void backtrace(uint32_t max_frames)
{
	print_header(max_frames);

	uint32_t *ebp = (uint32_t *)get_ebp();

	for (uint32_t i = 0; i < max_frames && ebp != nullptr; i++) {
		uint32_t ret_addr = ebp[1];

		if (ret_addr == 0) {
			break; // End of stack trace
		}

		uint32_t	offset = 0;
		const char *symbol = ksym_resolve(ret_addr, &offset);
		format_print(i, ret_addr, symbol, offset);

		ebp = (uint32_t *)ebp[0];
	}
}
