INCLUDES += -Iarch/i386

# --- Sources to be compiled ---

SRCS += arch/i386/io.c

# --- Subdirectories to be built ---

include arch/i386/gdt/module.mk
include arch/i386/stack/module.mk
