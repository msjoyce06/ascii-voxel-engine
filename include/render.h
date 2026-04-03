#ifndef RENDER_H
#define RENDER_H

#include "vectors.h"
#include "world.h"
#include "controller.h"

typedef struct camera camera_t;

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
void outline_block(camera_t *cam, veci_t block_pos, char c);
void highlight_selection(camera_t *cam);
void draw_crosshair(void);

#endif
