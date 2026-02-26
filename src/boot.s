/* Declare all necessary constant for Multiboot Header */

.set ALIGN,	1<<0 /* align loaded modules on page bondaries */
.set MEMINFO, 1 << 1 /* provide memory map info to OS */
.set FLAGS, ALIGN | MEMINFO /* set flags 'Mutiboot' */
.set MAGIC, 0x1BADB002 /* magic number for Multiboot Header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum for Multiboot Header */

/* Multiboot Header */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* 
 * Creation of Stack for the kernel
 *
 * Stack grows downwards, so we start from the top and move downwards.
 */

.section .bss
.align 16 /* 16 byte aligned stack for Complient of Sys V ABI */
.stack_bottom:
.skip 16384 /* 16KB stack */
.stack_top:

.section .text
.global __start
.type __start, @function
__start:
	/* Set up the stack pointer */
	mov .stack_top, %esp

	call kmain /* Call the kernel main function */

	/* If kmain returns, we will just halt the CPU */
.halt:
	hlt
	jmp .halt

.size __start, .-__start

