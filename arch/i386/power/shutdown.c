#include "shutdown.h"
#include "i386/io.h"
#include "pit.h"
#include "klibc/string/string.h"

// --- ACPI Table Structures ---

struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct acpi_sdt_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rsdt {
    struct acpi_sdt_header header;
    uint32_t tables[];
} __attribute__((packed));

struct acpi_fadt {
    struct acpi_sdt_header header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved;
    uint8_t  preferred_pm_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_req;
    uint8_t  pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;      // Offset 64
    uint32_t pm1b_cnt_blk;      // Offset 68
} __attribute__((packed));

// --- ACPI Helper Functions ---

static struct acpi_rsdp *acpi_find_rsdp(void) {
    uint16_t ebda_segment;
    __asm__ volatile("movw (0x40E), %0" : "=r"(ebda_segment));
    uint32_t ebda_start = ((uint32_t)ebda_segment) << 4;

    if (ebda_start >= 0x80000 && ebda_start < 0xA0000) {
        for (uint32_t addr = ebda_start; addr < ebda_start + 1024; addr += 16) {
            if (kstrncmp((const char *)addr, "RSD PTR ", 8) == 0) {
                return (struct acpi_rsdp *)addr;
            }
        }
    }

    for (uint32_t addr = 0x0E0000; addr < 0x100000; addr += 16) {
        if (kstrncmp((const char *)addr, "RSD PTR ", 8) == 0) {
            return (struct acpi_rsdp *)addr;
        }
    }

    return 0;
}

static struct acpi_fadt *acpi_find_fadt(struct acpi_rsdt *rsdt) {
    if (!rsdt) {
        return 0;
    }

    int entries = (int)((rsdt->header.length - sizeof(struct acpi_sdt_header)) / 4);

    for (int i = 0; i < entries; i++) {
        struct acpi_sdt_header *table = (struct acpi_sdt_header *)rsdt->tables[i];
        if (table && kstrncmp(table->signature, "FACP", 4) == 0) {
            return (struct acpi_fadt *)table;
        }
    }

    return 0;
}

static int acpi_extract_slp_types(struct acpi_fadt *fadt, uint16_t *slp_typa, uint16_t *slp_typb) {
    if (!fadt || !fadt->dsdt) {
        return -1;
    }

    const char *dsdt = (const char *)fadt->dsdt;
    uint32_t dsdt_length = *(const uint32_t *)(dsdt + 4);

    for (uint32_t i = 0; i < dsdt_length - 4; i++) {
        if (kstrncmp(&dsdt[i], "_S5_", 4) == 0) {
            const char *ptr = &dsdt[i + 4];
            if (*ptr == 0x12) { // PackageOp
                ptr++;
                if ((*ptr & 0xC0) == 0) {
                    ptr++;
                } else if ((*ptr & 0xC0) == 0x40) {
                    ptr += 2;
                } else if ((*ptr & 0xC0) == 0x80) {
                    ptr += 3;
                } else if ((*ptr & 0xC0) == 0xC0) {
                    ptr += 4;
                }
                ptr++; // Skip number of elements

                // SLP_TYPa
                if (*ptr == 0x0A) {
                    *slp_typa = (uint8_t)*(ptr + 1);
                    ptr += 2;
                } else if (*ptr == 0x00 || *ptr == 0x01) {
                    *slp_typa = (uint8_t)*ptr;
                    ptr++;
                } else {
                    *slp_typa = (uint8_t)*ptr;
                    ptr++;
                }

                // SLP_TYPb
                if (*ptr == 0x0A) {
                    *slp_typb = (uint8_t)*(ptr + 1);
                } else if (*ptr == 0x00 || *ptr == 0x01) {
                    *slp_typb = (uint8_t)*ptr;
                } else {
                    *slp_typb = (uint8_t)*ptr;
                }

                return 0;
            }
        }
    }

    return -1;
}

static void acpi_shutdown(void) {
    struct acpi_rsdp *rsdp = acpi_find_rsdp();
    if (!rsdp) {
        return;
    }

    struct acpi_rsdt *rsdt = (struct acpi_rsdt *)rsdp->rsdt_address;
    struct acpi_fadt *fadt = acpi_find_fadt(rsdt);
    if (!fadt) {
        return;
    }

    uint16_t slp_typa = 0;
    uint16_t slp_typb = 0;
    if (acpi_extract_slp_types(fadt, &slp_typa, &slp_typb) != 0) {
        return;
    }

    // Enable ACPI if not already enabled (bit 0 = SCI_EN in PM1a_CNT)
    if (fadt->smi_command_port != 0 && fadt->acpi_enable != 0) {
        if ((inw((uint16_t)fadt->pm1a_cnt_blk) & 1) == 0) {
            outb(fadt->acpi_enable, (uint16_t)fadt->smi_command_port);
            int timeout = 300;
            while (timeout > 0 && (inw((uint16_t)fadt->pm1a_cnt_blk) & 1) == 0) {
                pit_sleep_ms(10);
                timeout--;
            }
        }
    }

    __asm__ __volatile__("cli");

    uint16_t shutdown_a = (uint16_t)((slp_typa << 10) | 0x2000);
    outw(shutdown_a, (uint16_t)fadt->pm1a_cnt_blk);

    if (fadt->pm1b_cnt_blk != 0) {
        uint16_t shutdown_b = (uint16_t)((slp_typb << 10) | 0x2000);
        outw(shutdown_b, (uint16_t)fadt->pm1b_cnt_blk);
    }
}

// --- Public Interface ---

void arch_shutdown(void) {
    // 1. Try standard ACPI shutdown
    acpi_shutdown();

    // 2. Emulator-specific power-off fallbacks (QEMU, Bochs, VirtualBox)
    __asm__ __volatile__("cli");
    outw(0x2000, 0x604);   // QEMU default (PIIX4 PM1a_CNT)
    outw(0x2000, 0xB004);  // Bochs / older QEMU
    outw(0x3400, 0x4004);  // VirtualBox
    outw(0x31, 0x501);     // QEMU isa-debug-exit

    // 3. Fallback infinite halt loop
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

void arch_reboot(void) {
    __asm__ __volatile__("cli");

    // 1. ACPI / PCI reset control register (0xCF9)
    outb(0x02, 0xCF9);
    outb(0x06, 0xCF9);

    // 2. 8042 Keyboard controller pulse reset line (port 0x64, command 0xFE)
    uint8_t temp;
    do {
        temp = inb(0x64);
        if (temp & 1) {
            (void)inb(0x60);
        }
    } while (temp & 2);
    outb(0xFE, 0x64);

    // 3. Fallback: Triple fault via empty IDT
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) null_idt = {0, 0};
    __asm__ __volatile__("lidt %0; int3" : : "m"(null_idt));

    while (1) {
        __asm__ __volatile__("hlt");
    }
}
