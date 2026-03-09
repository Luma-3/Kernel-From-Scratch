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

/* Hardware text mode color constants. */
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

size_t terminal_row;
size_t terminal_column;


void terminal_putint(int value) {
  char buffer[12]; // Enough to hold -2^31 and null terminator
  int index = 0;
  bool is_negative = false;

  if (value < 0) {
    is_negative = true;
    value = -value;
  }

  do {
    buffer[index++] = '0' + (value % 10);
    value /= 10;
  } while (value > 0);

  if (is_negative) {
    buffer[index++] = '-';
  }

  // Reverse the buffer
  for (int i = index - 1; i >= 0; i--) {
    display_put_char(buffer[i]);
  }
}

void terminal_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    display_put_char(data[i]);
}

void terminal_writestring(const char *data) {
  terminal_write(data, strlen(data));
}


struct s_test {
  int a;
  long b;
};

void sleep(int seconds) {
  for (volatile int i = 0; i < seconds * 1000000; i++);
}

void kmain(void) {
  init_display(DRIVER_VGA);
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
  enum display_color color = COLOR_BLACK;
  enum display_color bg_color = COLOR_WHITE;
  while (1) {
    display_set_color(color, bg_color);
    color = (color + 1) % 16;
    bg_color = (bg_color + 1) % 16;
    sleep(1000);
  }
}
