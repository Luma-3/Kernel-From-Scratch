# ============================================
# 1. INITIALISATION & CONFIGURATION
# ============================================

include config.mk

.PHONY:	all clean mrproper re debug_mk iso run tidy tidy-fix lsp

all:	$(BIN_DIR)/$(TARGET)

# ============================================
# 2. Modules Inclution
# ============================================

SRCS		:=
INCLUDES	:= -I.

include boot/module.mk
include arch/i386/module.mk
include drivers/module.mk
include kernel/module.mk
include klibc/module.mk
include terminal/module.mk

# ============================================
# 3. Objects & Dependencies
# ============================================

OBJS	:= $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS	:= $(OBJS:.o=.d)

-include $(DEPS)

# ============================================
# 5. Generic Rules
# ============================================

$(BIN_DIR)/$(TARGET): $(OBJS) $(LDSCRIPT)
	@$(MKDIR) $(BIN_DIR)
	@$(ECHO) " $(COLOR_LD)[LD]$(COLOR_RESET) %s\n" $@
	@$(LD) $(LDFLAGS) -T $(LDSCRIPT) $(OBJS) -o $@

$(BUILD_DIR)/%.o: %.c
	@$(MKDIR) $(dir $@)
	@$(ECHO) " $(COLOR_CC)[CC]$(COLOR_RESET) %s\n" $<
	@$(CC) $(CFLAGS) $(COMFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@$(MKDIR) $(dir $@)
	@$(ECHO) " $(COLOR_AS)[AS]$(COLOR_RESET) %s\n" $<
	@$(AS) $(ASFLAGS) $< -o $@

# ============================================
# 6. Special Rules
# ============================================

GRUB_CFG	:= grub.cfg

ISO			:= $(BIN_DIR)/$(TARGET:.bin=.iso)
ISO_DIR		:= $(BIN_DIR)/iso

iso : $(ISO)

$(ISO): $(BIN_DIR)/$(TARGET) $(GRUB_CFG)
	@$(ECHO) " $(COLOR_LD)[ISO]$(COLOR_RESET) %s\n" $@
	@$(MKDIR) $(ISO_DIR)/boot/grub
	@cp $< $(ISO_DIR)/boot/$(TARGET)
	@cp $(GRUB_CFG) $(ISO_DIR)/boot/grub
	@$(GRUB_MKRESCUE) -o $@ $(ISO_DIR)

run: $(ISO)
	@qemu-system-i386 -cdrom $(ISO)

# ============================================
# 7. Clean Rules
# ============================================

clean:
	$(RM) $(BUILD_DIR)

mrproper:
	$(RM) $(BUILD_DIR) $(BIN_DIR)

re: mrproper all

# ============================================
# 8. Debug Rules
# ============================================

debug_mk:
	@echo "SRCS: $(SRCS)"
	@echo "OBJS: $(OBJS)"
	@echo "TARGET: $(TARGET)"
	@echo "AS Compiler: $(AS)"
	@echo "CC Compiler: $(CC)"
	@echo "INCLUDES:" $(INCLUDES)
	@echo "GRUB_MKRESCUE: $(GRUB_MKRESCUE)"
	@echo "LDSCRIPT: $(LDSCRIPT)"
	@echo "TIDY_SRCS: $(TIDY_SRCS)"

# ============================================
# 9. LSP Rules
# ============================================

lsp: clean
	@echo "Creating LSP file..."
	@bear -- $(MAKE) all

# ============================================
# 10. Analysis Rules
# ============================================

TIDY_SRCS := $(filter %.c,$(SRCS))

# all source without .s files
tidy: $(TIDY_SRCS)
	@echo "Running clang-tidy..."
	@clang-tidy $^ -- $(CFLAGS) $(INCLUDES)

tidy-fix: $(TIDY_SRCS)
	@echo "Running clang-tidy with fix..."
	@clang-tidy $^ --fix -- $(CFLAGS) $(INCLUDES)

