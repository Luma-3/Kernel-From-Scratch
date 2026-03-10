
#--- Standard Variables ---

SUBDIRS := drivers/input

#--- Object to be compiled ---

obj-y += $(SUBDIRS)/keyevent.o

#--- Sub directories ---

SUBDIRS := $(SUBDIRS)/ps2

include $(addsuffix /Makefile,$(SUBDIRS))
