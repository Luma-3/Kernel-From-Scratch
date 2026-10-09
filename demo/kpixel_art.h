#ifndef __KFS_KPIXEL_ART_H
# define __KFS_KPIXEL_ART_H
# include <stdint.h>

void draw_apple_vbe(uint32_t start_x, uint32_t start_y);
void draw_potato_vbe(uint32_t start_x, uint32_t start_y);
void draw_axelote_on_bucket_vbe(uint32_t start_x, uint32_t start_y);
void draw_saturn_vbe(uint32_t start_x, uint32_t start_y);

void draw_party_parrot_frame_vbe(uint32_t start_x, uint32_t start_y, uint32_t frame, uint32_t scale);
void draw_party_parrot_vbe(uint32_t start_x, uint32_t start_y);
void demo_party_parrot_vbe(uint32_t start_x, uint32_t start_y, uint32_t scale, uint32_t loops, uint32_t delay_ms);

#endif