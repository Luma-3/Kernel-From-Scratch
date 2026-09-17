INCLUDES	+= -Iterminal

# --- Object to be compiled ---

SRCS	+=	terminal/terminal.c	\
			terminal/ansii.c	\
			terminal/cursor.c


include terminal/font/module.mk
