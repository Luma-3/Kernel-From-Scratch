#ifndef KFS_ARCH_SHUTDOWN_H
#define KFS_ARCH_SHUTDOWN_H

#include <stdint.h>

/**
 * Shuts down the system using ACPI, falling back to emulator-specific power management ports.
 */
void arch_shutdown(void);

/**
 * Reboots the system using ACPI/PCI reset register, keyboard controller, and triple-fault fallback.
 */
void arch_reboot(void);

#endif // KFS_ARCH_SHUTDOWN_H
