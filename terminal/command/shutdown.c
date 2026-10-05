#include "shutdown.h"
#include "power/shutdown.h"
#include "printk.h"
#include "printf.h"
#include "pit.h"
#include "command.h"
#include "klibc/string/string.h"

static void extract_shutdown_counter(void *data, const uint8_t *src, size_t n) {
    if (src == NULL || data == NULL) {
        printk(KERNEL_LOG_LEVEL_ERROR, "Invalid arguments for extract_shutdown_counter\n");
        return;
    }
    (void)n;
    *((int32_t *)data) = katoi((const char *)src);
}

int32_t shutdown_command(int32_t argc, const uint8_t *argv) {
    int32_t count = 0;
    if (argc >= 1) {
        if (extract_arguments(&argv, (void *)&count, &extract_shutdown_counter) != 0) {
            printk(KERNEL_LOG_LEVEL_ERROR, "Failed to extract arguments for shutdown command\n");
            return -1;
        }

        for (int32_t i = 0; i < count; i++) {
            printf("\rAttempting to shutdown... (%d/%d)", i + 1, count);
            pit_sleep_ms(1000);
        }
        printf("\r");
    }

    printk(KERNEL_LOG_LEVEL_INFO, "Shutting down the system...\n");
    pit_sleep_ms(1000);
    arch_shutdown();
    return 0;
}