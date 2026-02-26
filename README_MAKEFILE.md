# Makefile Documentation

This document explains how the build system works for the **Kernel From Scratch** project.

---

## Overview

The build system uses a **recursive / modular Makefile** architecture, inspired by the Linux kernel build system. A root `Makefile` coordinates the overall build, while each subdirectory (`boot/`, `kernel/`) contains its own `Makefile` that contributes object files to the final binary.

---

## Prerequisites

Before running any build commands, make sure you have the following tools installed:

| Tool | Purpose |
|---|---|
| `i386-elf-gcc` | Cross-compiler for i386 bare-metal targets |
| `i386-elf-as` | Cross-assembler for i386 bare-metal targets |
| `grub-mkrescue` or `grub2-mkrescue` | Creates a bootable ISO image |
| `qemu-system-i386` | Emulator to run the kernel |

The cross-compiler toolchain is expected to be located at:
```
~/opt/cross/bin/i386-elf-gcc
~/opt/cross/bin/i386-elf-as
```

> **Note:** If the cross-compiler is not found at startup, the build will immediately fail with an error message.

---

## Directory Structure

```
.
├── Makefile          # Root Makefile (entry point)
├── grub.cfg          # GRUB configuration file (for ISO creation)
├── boot/
│   ├── Makefile      # Declares boot object files
│   ├── boot.s        # Assembly boot entry point
│   └── linker.ld     # Linker script
└── kernel/
    ├── Makefile      # Declares kernel object files
    └── kmain.c       # Kernel main C file
```

Compiled outputs are placed in:
- `obj/` — all `.o` object files (mirrors source directory structure)
- `bin/` — final kernel binary (`kernel.bin`)

---

## Toolchain & Flags

### Variables

| Variable | Value | Description |
|---|---|---|
| `CROSS_PATH` | `$(HOME)/opt/cross` | Path to the cross-compiler toolchain |
| `TARGET` | `i386-elf` | Target architecture |
| `CC` | `$(CROSS_PATH)/bin/i386-elf-gcc` | C compiler |
| `AS` | `$(CROSS_PATH)/bin/i386-elf-as` | Assembler |
| `LD` | `$(CROSS_PATH)/bin/i386-elf-gcc` | Linker (gcc used as linker frontend) |
| `CFLAGS` | `-std=gnu99 -ffreestanding -O2 -Wall -Wextra` | Compiler flags |
| `LDFLAGS` | `-ffreestanding -O2 -nostdlib` | Linker flags |

> `-ffreestanding` and `-nostdlib` are critical for kernel development: they disable the standard C library and runtime, which are not available in a bare-metal environment.

---

## How the Modular Build System Works

The build system uses a **single accumulator variable** `obj-y` to collect all object files across subdirectories.

1. The root `Makefile` initializes `obj-y` as empty.
2. It then `include`s the `Makefile` from each subdirectory listed in `SUBDIRS`:
   ```makefile
   SUBDIRS := boot kernel
   include $(addsuffix /Makefile,$(SUBDIRS))
   ```
3. Each subdirectory `Makefile` appends its own object files to `obj-y`:
   ```makefile
   # boot/Makefile
   obj-y += boot/boot.o

   # kernel/Makefile
   obj-y += kernel/kmain.o
   ```
4. Back in the root `Makefile`, all objects are prefixed with the `obj/` directory:
   ```makefile
   OBJS := $(addprefix $(OBJDIR)/,$(obj-y))
   # Result: obj/boot/boot.o obj/kernel/kmain.o
   ```

This pattern makes it easy to **add new modules**: just create a new subdirectory, add a `Makefile` with `obj-y += ...`, and add the directory to `SUBDIRS`.

---

## Build Rules

### Compiling C files

```makefile
$(OBJDIR)/%.o: %.c
    @mkdir -p $(dir $@)
    $(CC) $(CFLAGS) -c $< -o $@
```

Any `.c` source file is compiled into a corresponding `.o` file inside `obj/`, preserving the directory structure. The output directory is created automatically if it doesn't exist.

### Assembling `.s` files

```makefile
$(OBJDIR)/%.o: %.s
    @mkdir -p $(dir $@)
    $(AS) $< -o $@
```

Assembly source files (`.s`) are assembled similarly, placing the output in `obj/`.

### Linking the kernel

```makefile
$(BINDIR)/kernel.bin: $(OBJS)
    @mkdir -p $(dir $@)
    $(LD) -T $(LDSCRIPT) $(LDFLAGS) -o $@ $^
```

All object files are linked together using the linker script (`boot/linker.ld`) to produce the final `bin/kernel.bin` binary.

---

## ISO Creation

The Makefile can also create a bootable ISO image using GRUB:

```makefile
$(ISO): $(BINDIR)/kernel.bin $(GRUB_CFG)
    @mkdir -p $(ISODIR)/boot/grub
    cp $< $(ISODIR)/boot/kernel.bin
    cp $(GRUB_CFG) $(ISODIR)/boot/grub/
    $(GRUB_MKRESCUE) -o $@ $(ISODIR)
    rm -rf $(ISODIR)
```

The process:
1. Creates a temporary ISO directory structure (`isodir/boot/grub/`)
2. Copies `kernel.bin` and `grub.cfg` into it
3. Runs `grub-mkrescue` to generate `kfs.iso`
4. Cleans up the temporary directory

> **Note:** The Makefile auto-detects whether `grub-mkrescue` or `grub2-mkrescue` is available on your system. If neither is found, the build will fail immediately.

---

## Available Targets

| Target | Description |
|---|---|
| `make` / `make all` | Build the kernel binary (`bin/kernel.bin`) |
| `make iso` | Build the kernel and create the bootable ISO (`kfs.iso`) |
| `make run` | Build the ISO and launch it in QEMU (`qemu-system-i386`) |
| `make clean` | Remove the `obj/` and `bin/` directories |
| `make mrproper` | Run `clean` and also remove the ISO file (`kfs.iso`) |
| `make debug_mk` | Print key Makefile variables for debugging |

---

## Debugging the Build System

The `debug_mk` target prints all important variables at the time of execution:

```
make debug_mk
```

Example output:
```
SRCDIR:
OBJDIR: obj
BINDIR: bin
SUBDIRS: boot kernel
obj-y: boot/boot.o kernel/kmain.o
OBJS: obj/boot/boot.o obj/kernel/kmain.o
```

This is useful to verify that all object files are correctly collected before building.

---

## Adding a New Module

To add a new module (e.g., a `drivers/` directory):

1. Create the directory and its source files:
   ```
   drivers/
   └── vga.c
   ```
2. Create `drivers/Makefile`:
   ```makefile
   obj-y += drivers/vga.o
   ```
3. Add `drivers` to `SUBDIRS` in the root `Makefile`:
   ```makefile
   SUBDIRS := boot kernel drivers
   ```

That's all — the build system will automatically pick it up.
