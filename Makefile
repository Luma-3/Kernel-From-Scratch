CROSS_PATH := $(HOME)/opt/cross

TARGET := i386-elf

CC	:= $(CROSS_PATH)/bin/$(TARGET)-gcc
AS	:= $(CROSS_PATH)/bin/$(TARGET)-as
LD	:= $(CROSS_PATH)/bin/$(TARGET)-gcc

ifeq ($(wildcard $(CC)),)
$(error "Cross-compiler not found at $(CC). Please check CROSS_PATH.")
endif

CFLAGS	:= -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS	:= -ffreestanding -O2 -nostdlib

OBJDIR := obj
BINDIR := bin


SUBDIRS := boot kernel

obj-y := # List of object files to be built

# --- Aggregate subdirectories --- #

include $(addsuffix /Makefile,$(SUBDIRS))

OBJS := $(addprefix $(OBJDIR)/,$(obj-y))

# --- Rules --- #

all: $(BINDIR)/kernel.bin


$(BINDIR)/kernel.bin: $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) -T $(LDSCRIPT) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@) 
	$(AS) $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

debug_mk:
	@echo "SRCDIR: $(SRCDIR)"
	@echo "OBJDIR: $(OBJDIR)"
	@echo "BINDIR: $(BINDIR)"
	@echo "SUBDIRS: $(SUBDIRS)"
	@echo "obj-y: $(obj-y)"
	@echo "OBJS: $(OBJS)"

.PHONY: all clean debug_mk



GRUB_CFG := grub.cfg

GRUB_MKRESCUE := $(shell which grub-mkrescue || which grub2-mkrescue)

ifeq ($(shell which $(GRUB_MKRESCUE)),)
$(error "grub-mkrescue not found. Please install GRUB tools.")
endif


ISO := kfs.iso
ISODIR := isodir

iso: $(ISO)

$(ISO): $(BINDIR)/kernel.bin $(GRUB_CFG)
	echo "Creating ISO image $@..."
	@mkdir -p $(ISODIR)/boot/grub
	cp $< $(ISODIR)/boot/kernel.bin
	cp $(GRUB_CFG) $(ISODIR)/boot/grub/
	$(GRUB_MKRESCUE) -o $@ $(ISODIR)
	rm -rf $(ISODIR)

run: iso
	qemu-system-i386 -cdrom $(ISO)

