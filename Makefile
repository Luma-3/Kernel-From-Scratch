CROSS_PATH := $(HOME)/opt

TARGET := i386-elf

CC	:= $(CROSS_PATH)/bin/$(TARGET)-gcc
AS	:= $(CROSS_PATH)/bin/$(TARGET)-as
LD	:= $(CROSS_PATH)/bin/$(TARGET)-gcc

ifeq ($(wildcard $(CC)),)
$(error "Cross-compiler not found at $(CC). Please check CROSS_PATH.")
endif

CFLAGS	:= -std=gnu99 -ffreestanding -O2 -Wall -Wextra -MMD -MP
LDFLAGS	:= -ffreestanding -O2 -nostdlib

OBJDIR := obj
BINDIR := bin

MODULES := arch/i386 drivers kernel klibc boot

obj-y := # List of object files to be built

# --- Aggregate subdirectories --- #

include $(addsuffix /Makefile,$(MODULES))

OBJS := $(addprefix $(OBJDIR)/,$(obj-y))

# --- Include Header Files --- #

INCLUDES_DIRS := $(sort $(dir $(obj-y)))

INCLUDES := $(addprefix -I,$(INCLUDES_DIRS))

CFLAGS += $(INCLUDES)

-include $(OBJS:.o=.d)

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

mrproper: clean
	rm -f $(ISO)

debug_mk:
	@echo "SRCDIR: $(SRCDIR)"
	@echo "OBJDIR: $(OBJDIR)"
	@echo "BINDIR: $(BINDIR)"
	@echo "SUBDIRS: $(SUBDIRS)"
	@echo "obj-y: $(obj-y)"
	@echo "OBJS: $(OBJS)"

re: mrproper all

.PHONY: all clean debug_mk mrproper re

# --- Special Rules --- #

lsp: clean
	@echo "create compile commands database for lsp"
	bear -- $(MAKE) all


# --- ISO Creation --- #

GRUB_CFG := grub.cfg

GRUB_MKRESCUE := $(CROSS_PATH)/bin/grub-mkrescue

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

