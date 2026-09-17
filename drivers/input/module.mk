INCLUDES += -Idrivers/input

#--- Sources to be compiled ---

SRCS += drivers/input/keyevent.c

#--- Sub directories ---

include drivers/input/ps2/module.mk
