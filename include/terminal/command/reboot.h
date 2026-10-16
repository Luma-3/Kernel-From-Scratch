#ifndef KFS_REBOOT_H
#define KFS_REBOOT_H
#include <stdint.h>

int32_t reboot_command(int32_t argc, const uint8_t *argv);

#endif // KFS_REBOOT_H