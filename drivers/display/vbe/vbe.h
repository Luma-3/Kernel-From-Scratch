#ifndef __KFS_VBE_H
# define __KFS_VBE_H

#define VBE_FONT_WIDTH 8u
#define VBE_FONT_HEIGHT 16u
#define VBE_FONT_SOURCE_WIDTH 8u
#define VBE_FONT_SOURCE_HEIGHT 8u
#define VBE_FONT_SCALE_X (VBE_FONT_WIDTH / VBE_FONT_SOURCE_WIDTH)
#define VBE_FONT_SCALE_Y (VBE_FONT_HEIGHT / VBE_FONT_SOURCE_HEIGHT)

struct s_display;
struct s_display_boot_config;

void init_vbe(struct s_display *display, const struct s_display_boot_config *boot_config);


#endif