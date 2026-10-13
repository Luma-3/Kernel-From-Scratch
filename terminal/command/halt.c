#include "halt.h"

int32_t halt_command(int32_t argc, const uint8_t *argv) {
    (void)argc;
    (void)argv;
    __asm__ __volatile__("hlt");
    return 0;
}