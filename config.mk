#=======================================================================
# 1. TOOLS & COMPILER
#=======================================================================
CROSS_COMPILER_PATH	?= ./tools/cross/bin
ARCH				?= i386-elf

CC				:= $(CROSS_COMPILER_PATH)/$(ARCH)-gcc
AS				:= $(CROSS_COMPILER_PATH)/$(ARCH)-as
LD				:= $(CROSS_COMPILER_PATH)/$(ARCH)-gcc
GRUB_MKRESCUE	:= $(CROSS_COMPILER_PATH)/grub-mkrescue

RM			:= rm -rf
MKDIR		:= mkdir -p
ECHO		:= printf 

TARGET		?= kernel.bin

#=======================================================================
# 2. REPERTORIES
#=======================================================================
BUILD_DIR	:= build
BIN_DIR		:= bin

#=======================================================================
# 3. OPTIONS OF BUILD
#=======================================================================
BUILD ?= debug

ifeq ($(BUILD), debug)
    OPT_FLAGS := -O0 -g3 -DDEBUG
else ifeq ($(BUILD), release)
    OPT_FLAGS := -O2 -DNDEBUG
else
    $(error invalid BUILD Mode. Expected Values : debug, release)
endif

#=======================================================================
# 4. FLAGS OF COMPILATION & LINKING
#=======================================================================
CSTD		:= -std=gnu23 -ffreestanding 
WARNS		:= -Wall -Wextra -Wpedantic -Wshadow -Werror
FPICFLAGS	?= -fPIC

# Preprocesing flags' (commun C/C++) : inclusions of dependances .d
COMFLAGS	+= -MMD -MP

CFLAGS		+= $(CSTD) $(WARNS) $(OPT_FLAGS)
ASFLAGS		+= 
LDFLAGS		+= -ffreestanding $(OPT_FLAGS) -nostdlib


# =======================================================================
# 5. DISPLAY & DEBUG
# =======================================================================

COLOR ?= 1

ifeq ($(COLOR), 1)
	COLOR_CC	:= \033[0;32m
	COLOR_AS	:= \033[0;33m
	COLOR_LD	:= \033[0;34m
	COLOR_RESET	:= \033[0m
endif

