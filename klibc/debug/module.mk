INCLUDES += -Iklibc/debug

# --- Source files ---

SRCS += klibc/debug/printk.c		\
		klibc/debug/serial_print.c

# --- Subdirectories ---

include klibc/debug/trace/module.mk
