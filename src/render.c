#include "render.h"
#include "args.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
    int idxs[4];
    vector_t norm;
    char c;
} face_t;

static char *buff;
static float *zbuff;
static int bufflen;

static face_t faces[8] = { { .idxs = {0, 1, 3, 2}, .norm = {0, 0, -1}, .c = '$' },
                           { .idxs = {0, 1, 5, 4}, .norm = {0, -1, 0}, .c = '/' },
                           { .idxs = {0, 2, 6, 4}, .norm = {-1, 0, 0}, .c = '?' },
                           { .idxs = {1, 3, 7, 5}, .norm = {1,  0, 0}, .c = '#' },
                           { .idxs = {2, 3, 7, 6}, .norm = {0,  1, 0}, .c = '@' },
                           { .idxs = {4, 5, 7, 6}, .norm = {0,  0, 1}, .c = '+' } };

/** Printing */
void hide_cursor(void) {
    printf("\x1b[?25l");
}

void show_cursor(void) {
    printf("\x1b[?25h\n");
}

void clear_screen(void) {
    printf("\x1b[2J");
}

void to_top_left(void) {
    printf("\x1b[H");
}

void print_frame(void) {
    to_top_left();
    for (int i = 0; i < bufflen; i++) {
        putchar(buff[i]);
        if (i % WIDTH == WIDTH-1 && i != bufflen-1)
            printf("\n");
    }
    fflush(stdout);
}

/** Rendering */
void init_buffs(void) {
    bufflen = WIDTH*HEIGHT;
    buff = malloc(bufflen);
    zbuff = malloc(bufflen * sizeof(float));
    if (buff == NULL || zbuff == NULL) {
        free(buff);
        free(zbuff);
        buff = NULL;
        zbuff = NULL;
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
}

void set_buffs(void) {
    for (int i = 0; i < bufflen; i++) {
        buff[i] = ' ';
        zbuff[i] = -INFINITY;
    }
}

void free_buffs(void) {
    free(buff);
    free(zbuff);
}

coord_t screen_proj(vector_t cam_space) {
    float f = HEIGHT / (2.0f * tanf(FOV/2.0f));
    float ooz = 1.0f / cam_space.z;

    float screen_x = (WIDTH/2.0f + 2.0f * f * cam_space.x * ooz);
    float screen_y = (HEIGHT/2.0f - f * cam_space.y * ooz);

    return (coord_t){screen_x, screen_y, ooz};
}

static float edge(coord_t v1, coord_t v2, coord_t p) {
    return (p.x - v1.x) * (v2.y - v1.y) - (p.y - v1.y) * (v2.x - v1.x);
}

void buffer_proj(coord_t p, char c) {
    if (0 <= p.x && p.x < WIDTH && 0 <= p.y && p.y < HEIGHT) {
        int idx = (int)p.y * WIDTH + (int)p.x;
        if (p.ooz > zbuff[idx]) {
            zbuff[idx] = p.ooz;
            buff[idx] = c;
        }
    }
}

void render_poly(coord_t s0, coord_t s1, coord_t s2, char c) {
    int left   = MAX(0,      (int)floorf(MIN(s0.x, MIN(s1.x, s2.x))));
    int right  = MIN(WIDTH,  (int)ceilf(MAX(s0.x, MAX(s1.x, s2.x))));
    int top    = MAX(0,      (int)floorf(MIN(s0.y, MIN(s1.y, s2.y))));
    int bottom = MIN(HEIGHT, (int)ceilf(MAX(s0.y, MAX(s1.y, s2.y))));

    float area = edge(s0, s1, s2);
    const float eps = 1e-6f;
    if (fabsf(area) < eps)
        return;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            coord_t p = {x + 0.5f, y + 0.5f, 0};

            float w0 = edge(s1, s2, p);
            float w1 = edge(s2, s0, p);
            float w2 = edge(s0, s1, p);

            if ((w0 >= -eps && w1 >= -eps && w2 >= -eps) ||
                (w0 <= eps && w1 <= eps && w2 <= eps)) {

                p.ooz = (w0/area)*s0.ooz + (w1/area)*s1.ooz + (w2/area)*s2.ooz;
                buffer_proj(p, c);
            }
        }
    }
}

void render_face(camera_t *cam, vector_t rel, face_t face) {
    return;
}

void render_block(camera_t *cam, vector_t world) {
    vector_t rel = v_sub(world, cam->pos);
    vector_t vtxs[8];
    for (int i = 0; i < 8; i++) {
        vtxs[i].x = (i & 1) - 0.5f + rel.x;
        vtxs[i].y = ((i >> 1) & 1) - 1.0f + rel.y;
        vtxs[i].z = ((i >> 2) & 1) - 0.5f + rel.z ;
    }
    // face_t faces[6];
    // faces[0] = (face_t){ .idxs = {0, 1, 3, 2}, .norm = {0, 0, -1}, .c = '$' };
    // faces[1] = (face_t){ .idxs = {0, 1, 5, 4}, .norm = {0, -1, 0}, .c = '/' };
    // faces[2] = (face_t){ .idxs = {0, 2, 6, 4}, .norm = {-1, 0, 0}, .c = '?' };
    // faces[3] = (face_t){ .idxs = {1, 3, 7, 5}, .norm = {1,  0, 0}, .c = '#' };
    // faces[4] = (face_t){ .idxs = {2, 3, 7, 6}, .norm = {0,  1, 0}, .c = '@' };
    // faces[5] = (face_t){ .idxs = {4, 5, 7, 6}, .norm = {0,  0, 1}, .c = '+' };

    for (int i = 0; i < 6; i++) {
        render_face(cam, rel, faces[i]);
    }
}

void render_chunk(camera_t *cam, const chunk_t *chunk) {
    vector_t offset = chunk->offset;
    for (int y = 0; y < CHUNK_Y; y++) {
        for (int z = 0; z < CHUNK_Z; z++) {
            for (int x = 0; x < CHUNK_Y; x++) {
                if (block_present(chunk, x, y, z))
                    vector_t world = {x + offset.x, y + offset.y, z + offset.z};
                    render_block(cam, world);
            }
        }
    }
}

// block_t place_block(vector_t pos) {
    // block_t block;
    // for (int i = 0; i < 8; i++) {
        // block.vtxs[i].x = (i & 1) - 0.5f + pos.x;
        // block.vtxs[i].y = ((i >> 1) & 1) - 2 + pos.y;
        // block.vtxs[i].z = ((i >> 2) & 1) - 0.5f + pos.z;
    // }

    // block.faces[0] = (face_t){ .idxs = {0, 1, 3, 2}, .norm = {0, 0, -1}, .c = '$' };
    // block.faces[1] = (face_t){ .idxs = {0, 1, 5, 4}, .norm = {0, -1, 0}, .c = '/' };
    // block.faces[2] = (face_t){ .idxs = {0, 2, 6, 4}, .norm = {-1, 0, 0}, .c = '?' };
    // block.faces[3] = (face_t){ .idxs = {1, 3, 7, 5}, .norm = {1,  0, 0}, .c = '#' };
    // block.faces[4] = (face_t){ .idxs = {2, 3, 7, 6}, .norm = {0,  1, 0}, .c = '@' };
    // block.faces[5] = (face_t){ .idxs = {4, 5, 7, 6}, .norm = {0,  0, 1}, .c = '+' };

    // return block;
// }

void draw_crosshair(void) {
    coord_t center = {WIDTH/2, HEIGHT/2, INFINITY};
    buffer_proj(center, '+');
    if (WIDTH > 300) {
        coord_t left = {center.x-1, center.y, INFINITY};
        coord_t right = {center.x+1, center.y, INFINITY};
        buffer_proj(left, '<');
        buffer_proj(right, '>');
    }
}
