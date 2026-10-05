# ifndef KFS_COMMAND_H
# define KFS_COMMAND_H

#include <stdint.h>

struct command {
    const uint8_t *name;
    const uint8_t *description;
    int32_t (*handler)(int32_t argc, const uint8_t *argv);
};

struct argument {
    uint32_t start_arg_index;
    uint32_t end_arg_index;
};

typedef struct command command_t;

typedef struct argument argument_t;

void register_command(const uint8_t *name, const uint8_t *description,
                      int32_t (*handler)(int32_t argc, const uint8_t *argv));

void init_command_system();

void execute_command(const uint8_t *command_line);

command_t *list_commands(uint32_t *count);

#endif // KFS_COMMAND_H