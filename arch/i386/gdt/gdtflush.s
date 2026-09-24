.global gdt_flush

gdt_flush:

	mov 4(%esp), %eax		; /* Get the GDT descriptor address from the stack */
	lgdt (%eax)				; /* Load the GDT descriptor into the GDTR register */

	mov $0x10, %ax			; /* Load the data segment (Kernel Data index 2 = 0x10) selector (0x10) into AX */
	mov %ax, %ds			; /* Update the DS register with the new data segment selector */
	mov %ax, %es			; /* Update the ES register with the new data segment selector */
	mov %ax, %fs			; /* Update the FS register with the new data segment selector */
	mov %ax, %gs			; /* Update the GS register with the new data segment selector */
	mov $0x18, %ax			; /* Load the stack segment (Kernel Stack index 3 = 0x18) selector (0x18) into AX */
	mov %ax, %ss			; /* Update the SS register with the new data segment selector */

	jmp $0x08, $flush		; /* Jump to the code segment (Kernel Code index 1 = 0x08) selector (0x08) to flush the instruction pipeline */

flush:
	ret
