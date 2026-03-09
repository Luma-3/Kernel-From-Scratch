/* Declare all necessary constant for Multiboot Header */

.set ALIGN,	1<<0 /* align loaded modules on page bondaries */
.set MEMINFO, 1 << 1 /* provide memory map info to OS */
.set VIDEO, 1 << 2 /* request video mode */
.set FLAGS, ALIGN | MEMINFO | VIDEO  /* set flags 'Mutiboot' */
.set MAGIC, 0x1BADB002 /* magic number for Multiboot Header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum for Multiboot Header */

/* Multiboot Header */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long	0
.long	0
.long	0
.long	0
.long	0
.long 0 /* mode_type: 0 = graphics */
.long 1920 /* width */
.long 1080 /* height */
.long 32 /* depth */

/* 
 * Creation of Stack for the kernel
 *
 * Stack grows downwards, so we start from the top and move downwards.
 */

.section .bss
.align 16 /* 16 byte aligned stack for Complient of Sys V ABI */
stack_bottom:
.skip 16384 /* 16KB stack */
stack_top:

.section .text
.global _start
.type _start, @function
_start:
	/* Set up the stack pointer */
	mov $stack_top, %esp

	/* Multiboot v1 ABI:
	 * EAX = magic (0x2BADB002)
	 * EBX = pointer to multiboot_info
	 */
	push %ebx
	push %eax
	call kmain /* Call the kernel main function */
	add $8, %esp

	/* If kmain returns, we will just halt the CPU */
	cli /* Clear interrupts */
1:	hlt /* Halt the CPU */
	jmp 1b /* Infinite loop */

.size _start, .-_start

