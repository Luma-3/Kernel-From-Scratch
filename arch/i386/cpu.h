#ifndef KFS_CPU_H
#define KFS_CPU_H

/**
 * @brief Clears the interrupt flag (disables maskable hardware interrupts).
 */
static inline __attribute__((always_inline)) void cli(void) {
    __asm__ volatile("cli");
}

/**
 * @brief Sets the interrupt flag (enables maskable hardware interrupts).
 */
static inline __attribute__((always_inline)) void sti(void) {
    __asm__ volatile("sti");
}

/**
 * @brief Halts the CPU until the next external interrupt.
 */
static inline __attribute__((always_inline)) void hlt(void) {
    __asm__ volatile("hlt");
}

#endif // KFS_CPU_H
