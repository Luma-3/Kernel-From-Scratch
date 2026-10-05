INCLUDES	+= -Iterminal -Icommand

# --- Object to be compiled ---

SRCS	+=	terminal/terminal.c	\
			terminal/ansii.c	\
			terminal/cursor.c   \
			terminal/command.c


include terminal/font/module.mk

include terminal/command/module.mk