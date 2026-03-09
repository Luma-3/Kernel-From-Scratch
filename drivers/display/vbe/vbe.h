#ifndef __KFS_VBE_H
# define __KFS_VBE_H


#define PACK_COLOR(r, g, b, config) \
     (((uint32_t)(r) & (config)->color_info.rgb.framebuffer_red_mask_size)  << (16 - (config)->color_info.rgb.framebuffer_red_field_position)) | \
      (((uint32_t)(g) & (config)->color_info.rgb.framebuffer_green_mask_size) << 16 -  ((config)->color_info.rgb.framebuffer_green_field_position)) | \
      (((uint32_t)(b) & (config)->color_info.rgb.framebuffer_blue_mask_size) << 16 -  ((config)->color_info.rgb.framebuffer_blue_field_position))
typedef struct s_display display_t;
typedef struct s_display_boot_config display_boot_config_t;

void init_vbe(display_t *display, const display_boot_config_t *boot_config);


#endif