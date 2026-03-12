#ifndef RENDER_H
#define RENDER_H

#include "vectors.h"

typedef struct {
    float x;
    float y;
    float ooz;
} coord_t;

void hide_cursor(void);

void show_cursor(void);

void clear_screen(void);

void to_top_left(void);

void print_frame(void);

void init_buffs(void);

void set_buffs(void);

void free_buffs(void);

coord_t screen_proj(vector_t cam_space);

void buffer_proj(coord_t p, char c);

void render_poly(coord_t s0, coord_t s1, coord_t s2, char c);

void draw_crosshair(void);

#endif
