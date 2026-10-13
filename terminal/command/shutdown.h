#ifndef KFS_SHUTDOWN_H
#define KFS_SHUTDOWN_H

#include <stdint.h>

int32_t shutdown_command(int32_t argc, const uint8_t *argv);

#endif // KFS_SHUTDOWN_H