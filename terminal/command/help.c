#include "help.h"
#include "terminal/command.h"
#include <stddef.h>
#include "klibc/utility/printf.h"

int32_t help_command(int32_t argc, const uint8_t *argv) {
    (void)argc;
    (void)argv;
    uint32_t command_count = 0;
    command_t *cmds = list_commands(&command_count);
    for (uint32_t i = 0; i < command_count && cmds[i].name != nullptr; i++) {
        if (cmds[i].description == nullptr) {
            printf("%s: <corrupted>\n", cmds[i].name);
            continue;
        }
        printf("%s: %s\n", cmds[i].name, cmds[i].description);
    }

    return 0;
}