LDSCRIPT := boot/linker.ld

INCLUDES += -Iboot

# --- Sources to be compiled ---

SRCS += boot/boot.s

# --- Subdirectories to be built ---

include boot/gdt/module.mk
