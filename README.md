# Kernel-From-Scratch 💻

An operating system kernel built from the ground up to understand the inner workings of x86 architecture, systems, and hardware communication.

## 📌 Project Structure

This project is divided into 10 sections by 42 school. The current goal is to complete sections 1 to 4 with [@SenseiTarzan](https://github.com/SenseiTarzan). 

* **Status:** Part 1 is now **Completed**! 🎉

---

## 🛠️ Features (Part 1)

- [x] Multiboot Compliance
- [x] VBE Driver with:
  - [x] Pixel color drawing
  - [x] Bitmap drawing
  - [x] Glyph drawing
- [x] Keyboard Polling System (PS/2)
- [x] Basic Terminal with:
  - [x] Auto-scrolling
  - [x] 3-window context switching
  - [x] Special event handling (Shift, Alt, Ctrl)

---

## 🚀 Roadmap & Planned Features

### 📅 Part 2
- [ ] Recreate the GDT from Multiboot
- [ ] Implement a minimalist shell
  - [ ] Basic commands (`echo`, `test`, `status`)
  - [ ] Utility commands (`halt`, `reboot`)

### 📅 Part 3 (To be refined)
- [ ] Complete memory code structure with paging handling
- [ ] Read and Write memory permissions
- [ ] User space and Kernel space memory separation
- [ ] Physical and Virtual memory management
- [ ] Physical memory helpers (`kmalloc`, `kfree`, `ksize`, `kbrk`)
- [ ] Virtual memory helpers (`vmalloc`, `vfree`, `vsize`, `vbrk`)
- [ ] Kernel Panic handling

### 📅 Part 4
- [ ] Create, fill, and register an Interrupt Descriptor Table (IDT)
- [ ] Signal-callback system on the Kernel API
- [ ] Signal scheduling interface
- [ ] Clean registers interface before a panic / halt
- [ ] Stack saving interface before a panic
- [ ] Upgrade Keyboard Polling to the IDT system

---

## 🏃 How to Run

### Prerequisites
Requires `qemu` and `make`. All other necessary cross-compilation tools are built directly via our toolchain script.

### Installation & Execution
This project includes a shell script to build the toolchain. Run it once before building the kernel:

```bash
# Build the cross-compilation toolchain (This might take a while, you have time to go make a coffee ☕)
./tools/build_toolchain.sh

# Compile and launch the kernel in QEMU
make run
```
