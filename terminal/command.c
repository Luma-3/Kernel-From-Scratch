#include "command.h"
#include "klibc/string/string.h"
#include "klibc/memory/mem.h"
#include "command/reboot.h"
#include "command/halt.h"
#include "command/help.h"
#include "printk.h"
#include <stddef.h>

struct command commands[256];

uint32_t command_count;

void register_command(const uint8_t *name, const uint8_t *description,
                      int32_t (*handler)(int32_t argc, const uint8_t *argv)) {
    if (command_count >= 256) {
        return;
    }
    commands[command_count].name = name;
    commands[command_count].description = description;
    commands[command_count].handler = handler;
    command_count++;
}


command_t *list_commands(uint32_t *count) {
    *count = command_count;
    return commands;
}

void init_command_system() {
    kmemset(commands, 0, sizeof(commands));
    command_count = 0;

    register_command((const uint8_t *)"reboot", (const uint8_t *)"Reboot the system", &reboot_command);
    register_command((const uint8_t *)"halt", (const uint8_t *)"Halt the system", &halt_command);
    register_command((const uint8_t *)"help", (const uint8_t *)"Display help information", &help_command);
}

command_t *find_command(const uint8_t *name) {
    char *cmd_name = (char *)name;
    uint32_t cmd_len = kstrlen(cmd_name);
    for (uint32_t i = 0; i < command_count; i++) {
        char *registered_name = (char *)commands[i].name;
        if (kstrlen(registered_name) != cmd_len) {
            continue;
        }
        if (kstrncmp((char *)commands[i].name, cmd_name, cmd_len) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

void execute_command(const uint8_t *command_line) {
    command_t *cmd = find_command(command_line);
    if (cmd == NULL) {
        printk(KERNEL_LOG_LEVEL_ERROR, "Command not found: %s\n", command_line);
        return;
    }
    printk(KERNEL_LOG_LEVEL_INFO, "Executing command: %s\n", command_line);
    cmd->handler(1, command_line);
}