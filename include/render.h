#ifndef RENDER_H
#define RENDER_H

#include "vectors.h"
#include "world.h"
#include "controller.h"

typedef struct {
    float x;
    float y;
    float ooz;
} coord_t;

/** printing */
void hide_cursor(void);
void show_cursor(void);
void clear_screen(void);
void to_top_left(void);
void print_frame(void);

/** memory */
void init_buffs(void);
void set_buffs(void);
void free_buffs(void);

/** rendering */
void render_chunks(camera_t *cam, const chunk_t chunks[], int num_chunks);
void draw_crosshair(void);

#endif
