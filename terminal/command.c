#include "command.h"
#include "klibc/string/string.h"
#include "klibc/memory/mem.h"
#include "command/reboot.h"
#include "command/halt.h"
#include "command/help.h"
#include "printk.h"
#include "printf.h"
#include "kdebug.h"
#include "klibc/kunistd.h"
#include <stddef.h>

struct command commands[256];

uint32_t command_count;

void register_command(const uint8_t *name, const uint8_t *description,
                      int32_t (*handler)(int32_t argc, const uint8_t *argv)) {
    if (command_count >= 256) {
        print_serial("Command registration failed: maximum command limit reached");
        return;
    }
    if(name == NULL || description == NULL || handler == NULL) {
        print_serial("Invalid command registration: name, description, and handler must not be NULL");
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

static uint8_t *jump_name(const uint8_t *str) {
    if (str == NULL) {
        return NULL;
    }
    while (*str != '\0' && *str != ' ' && *str != '\t') {
        str++;
    }
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    if (*str == '\0') {
        return NULL;
    }
    return (uint8_t *)str;
}

static uint32_t len_name(const uint8_t *str) {
    uint32_t len = 0;
    while (str[len] != '\0' && str[len] != ' ' && str[len] != '\t') {
        len++;
    }
    return len;
}

int8_t extract_arguments(const uint8_t **argv, void *data, void (*method_copy)(void *dest, const uint8_t *src, size_t n)) {
    if (argv == NULL || data == NULL || method_copy == NULL) {
        print_serial("Invalid arguments for extract_arguments: argv, data, and copy_data must not be NULL");
        return FAILLURE;
    }
    const uint8_t *arg_start = *argv;
    while (*arg_start != '\0') {
        while (*arg_start == ' ' || *arg_start == '\t') {
            arg_start++;
        }
        if (*arg_start == '\0') {
            break; // No more arguments
        }
        const uint8_t *arg_end = arg_start;
        while (*arg_end != '\0' && *arg_end != ' ' && *arg_end != '\t') {
            arg_end++;
        }
        size_t arg_len = arg_end - arg_start;
        method_copy(data, arg_start, arg_len);
        *argv = arg_end;
        break;
    }
    return SUCCESS;
}

static int32_t count_argc(const uint8_t *str) {
    if (str == NULL) {
        return 0;
    }
    int32_t argc = 0;
    const uint8_t *ptr = str;
    while (*ptr != '\0') {
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }
        if (*ptr != '\0') {
            argc++;
            while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t') {
                ptr++;
            }
        }
    }
    return argc;
}

command_t *find_command(const uint8_t *name) {
    if (name == NULL) {
        return NULL;
    }
    const char *cmd_name = (const char *)name;
    const uint32_t cmd_len = len_name(name);
    for (uint32_t i = 0; i < command_count; i++) {
        char *registered_name = (char *)commands[i].name;
        if (kstrlen(registered_name) != cmd_len) {
            continue;
        }
        if (kstrncmp(registered_name, cmd_name, cmd_len) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

void execute_command(const uint8_t *command_line) {
    if(command_line == NULL) {
        printk(KERNEL_LOG_LEVEL_ERROR, "Command line is NULL\n");
        return;
    }
    command_t *cmd = find_command(command_line);
    if (cmd == NULL) {
        printk(KERNEL_LOG_LEVEL_ERROR, "Command not found: %s\n", command_line);
        return;
    }
    const uint8_t *args = jump_name(command_line);
    int32_t argc = count_argc(args);
    cmd->handler(argc, args);
}