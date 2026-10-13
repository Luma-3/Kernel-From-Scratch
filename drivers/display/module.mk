
INCLUDES += -Idrivers/display

SRCS += drivers/display/display.c

#--- Sub directories ---

include drivers/display/vbe/module.mk
include drivers/display/vga/module.mk
