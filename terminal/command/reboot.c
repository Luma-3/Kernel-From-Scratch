#include "reboot.h"
#include "i386/io.h"
#include "printk.h"
#include "printf.h"
#include "klibc/string/string.h"
#include "pit.h"
#include "command.h"

void extract_counter( void *data, const uint8_t *src, size_t n) {
    if (src == NULL || data == NULL) {
        printk(KERNEL_LOG_LEVEL_ERROR, "Invalid arguments for extract_counter: src and data must not be NULL");
        return;
    }
    (void)n;
    *((int32_t *)data) = katoi((const char *)src);
}

int32_t reboot_command(int32_t argc, const uint8_t *argv) {
    if(argc < 1) {
        printf("Usage: reboot <count>\n");
        return -1;
    }
    int32_t count;
    if (extract_arguments(&argv, (void *)&count, &extract_counter) != 0) {
        printk(KERNEL_LOG_LEVEL_ERROR, "Failed to extract arguments for reboot command\n");
        return -1;
    }
    printk(KERNEL_LOG_LEVEL_INFO, "Reboot command received with count: %d\n", count);
    for(int32_t i = 0; i < count; i++) {
        printf("\rAttempting to reboot... (%d/%d)", i + 1, count);
        pit_sleep_ms(1000);
    }
    printf("\r");
    printk(KERNEL_LOG_LEVEL_INFO, "Rebooting the system... \n");
    pit_sleep_ms(1500);
    outb(0x06, 0xCF9); // method modern pour forcer le redémarrage du système par ACPI Reset Control Register
    return 0;
}