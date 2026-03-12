#--- Standard Variables ---

DIR := terminal

# --- Object to be compiled ---

obj-y += $(DIR)/terminal.o
obj-y += $(DIR)/ansii.o

#--- Sub directories ---

SUBDIRS := $(DIR)/font

include $(addsuffix /Makefile,$(SUBDIRS))
