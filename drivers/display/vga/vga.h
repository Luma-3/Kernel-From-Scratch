//
// Created by gcaptari on 2/27/26.
//

#ifndef __KFS_VGA_H
# define __KFS_VGA_H

# include <stdint.h>

# define VGA_WIDTH 80
# define VGA_HEIGHT 25
# define VGA_MEMORY 0xB8000
# define VGA_MERGE_COLOR(fg, bg) ((bg << 4) | (fg))
# define VGA_ENTRY(c, color) ((uint16_t)(c) | (uint16_t)(color) << 8)
# define VGA_EXTRACT_CHAR(entry) ((char)(entry & 0xFF))
# define VGA_EXTRACT_COLOR(entry) ((uint8_t)((entry >> 8) & 0xFF))


typedef struct s_display display_t;

void init_vga(display_t *display);


#endif //__KFS_VGA_H