#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

void kmain(void) {
  while (1) {
    // Infinite loop to keep the kernel running
  }
}
