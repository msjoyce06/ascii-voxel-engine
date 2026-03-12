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

void render_face(vector_t v0, vector_t v1, vector_t v2, vector_t v3, vector_t cam_pos,
                 float cost, float sint, float cosp, float sinp, char c);

void render_chunk(const chunk_t *chunk, vector_t cam_pos,
                  float cost, float sint, float cosp, float sinp);

void draw_crosshair(void);

#endif
