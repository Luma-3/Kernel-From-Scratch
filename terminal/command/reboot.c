#include "reboot.h"
#include "i386/io.h"
#include "printk.h"

int32_t reboot_command(int32_t argc, const uint8_t *argv) {
    (void)argc;
    (void)argv;
    struct {
        unsigned short limit;
        unsigned int base;
    } __attribute__((packed)) invalid_idt = {0, 0};

    __asm__ __volatile__ (
        "lidt %0\n\t" 
        "int $3"
        :
        : "m"(invalid_idt)
    ); // method tripple fault pour forcer le redémarrage du système
    printk(KERNEL_LOG_LEVEL_INFO, "Reboot command issued.\n");
    return 0;
}