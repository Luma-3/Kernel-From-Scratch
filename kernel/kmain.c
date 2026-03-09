#if defined(__linux__)
#error "This code is not meant to be compiled on Linux."
#endif

#if !defined(__i386__)
#error "This code must be compiled with a 32-bit target."
#endif

#include <stdint.h>
#include <stddef.h>
#include "../klibc/string/string.h"
#include "../klibc/math/math.h"
#include "../klibc/memory/mem.h"
#include "../drivers/display/display.h"
#include "../drivers/input/ps2/ps2.h"
#include "../terminal/terminal.h"

struct s_test {
  int a;
  long b;
};

void kmain(void) {
  init_display(DRIVER_VGA);
  init_terminal("Main Terminal");
  char hello[] = "Hello, kernel World!\n";
  if (k_strcmp(hello, "Hello World!\n") != 0) {
      terminal_writestring("k_strcmp worked correctly.\n");
  }
  if (k_strncmp(hello, "Hello, kernel", 5) == 0) {
      terminal_writestring("k_strncmp worked correctly.\n");
  }
  if (k_strrchr(hello, '\n') == (hello + (k_strrchr(hello, '\n') - hello))) {
    terminal_writestring("k_strrchr worked correctly.\n");
  }
  if (k_strchr(hello, ',') == (hello + (k_strchr(hello, ',') - hello))) {
    terminal_writestring("k_strchr worked correctly.\n");
  }

  const int a = -5;
  const int b = 10;
  if (k_abs(a) == 5) {
      terminal_writestring("k_abs worked correctly.\n");
  }
  if (k_max(a, b) == b) {
      terminal_writestring("k_max worked correctly.\n");
  }
  if (k_min(a, b) == a) {
      terminal_writestring("k_min worked correctly.\n");
  }
  const long x = -100L;
  const long y = 200L;
  if (k_labs(x) == 100L) {
      terminal_writestring("k_labs worked correctly.\n");
  }
  if (k_lmax(x, y) == y) {
      terminal_writestring("k_lmax worked correctly.\n");
  }
  if (k_lmin(x, y) == x) {
      terminal_writestring("k_lmin worked correctly.\n");
  }

  struct s_test test_struct = {42, 123456789L};
  k_memset(&test_struct, 0, 4);
  if (test_struct.a == 0 && test_struct.b == 123456789L) {
      terminal_writestring("k_memset worked correctly.\n");
  }

  terminal_writestring(hello);
}
