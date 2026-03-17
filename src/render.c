#include "render.h"
#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
    int x;
    int y;
    float ooz;
    vecf_t cam_space;
} pixel_t;

typedef enum {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    TOP,
    BOTTOM
} face_dir_t;

typedef struct {
    int idxs[4];
    face_dir_t dir;
} face_t;

static char *buff;
static float *zbuff;
static int bufflen;

const float EPS = 1e-6f;
const float NEAR_PLANE = 0.001f;
const float EDGE_WIDTH = 0.025f;

static vecf_t ref_vtxs[8] = {
    {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}
};

static face_t ref_faces[6] = {
    { .idxs = {4, 0, 3, 7}, WEST },   // x = 0
    { .idxs = {1, 5, 6, 2}, EAST },   // x = 1
    { .idxs = {0, 1, 2, 3}, SOUTH },  // z = 0
    { .idxs = {5, 4, 7, 6}, NORTH },  // z = 1
    { .idxs = {4, 5, 1, 0}, BOTTOM }, // y = 0
    { .idxs = {3, 2, 6, 7}, TOP }     // y = 1
};

/** printing */
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

/** memory */
void init_buffs(void) {
    bufflen = WIDTH*HEIGHT;
    buff = malloc(bufflen);
    zbuff = malloc(bufflen * sizeof(float));
    if (buff == NULL || zbuff == NULL) {
        free(buff);
        free(zbuff);
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

/** rendering */
static float edge(pixel_t v1, pixel_t v2, pixel_t p) {
    return (p.x - v1.x)*(v2.y - v1.y) - (p.y - v1.y)*(v2.x - v1.x);
}

// static int should_draw_edge(pixel_t s0, pixel_t s1, vecf_t v0, vecf_t v1) {
    // float dx = fabsf(s1.x - s0.x);
    // float dy = fabsf(s1.y - s0.y);
    // float dist = sqrtf(v0.x*v1.x + v0.y*v1.y + v0.z*v1.z);

    // if (dist > 8.0f && dy < 0.1f*dx)
        // return 0;
    // return 1;
// }

// static char get_shade_char(vecf_t norm, float dist) {
    // int levels;
    // char *ramp;
    // if (WIDTH < 250) {
        // ramp = " -+?l$%@";
        // levels = 8;
    // } else {
        // ramp = " -:+!?ilJ$KX%0#@";
        // levels = 16;
    // }
    // float base_shade;

    // if (norm.y == 1.0f)       // top
        // base_shade = 1.1f;
    // else if (norm.z == -1.0f) // south
        // base_shade = 0.8f;
    // else if (norm.x == -1.0f) // west
        // base_shade = 0.65f;
    // else if (norm.z == 1.0f)  // north
        // base_shade = 0.55f;
    // else if (norm.x == 1.0f)  // east
        // base_shade = 0.45f;
    // else                      // bottom
        // base_shade = 0.25f;

    // float near = 0.5f;
    // float far = 18.0f;
    // float d = 1.0f - (dist - near) / (far - near);
    // if (d < 0.0f) d = 0.0f;
    // if (d > 1.0f) d = 1.0f;

    // float brightness = base_shade * d;
    // int idx = (int)(brightness * (levels - 1) + 0.5f);
    // if (idx < 0) idx = 0;
    // if (idx > levels - 1) idx = levels - 1;

    // return ramp[idx];
// }

static char get_shade_char(face_dir_t dir) {
    switch (dir) {
        case NORTH:
            return '$';
        case EAST:
            return '?';
        case SOUTH:
            return '$';
        case WEST:
            return '?';
        case TOP:
            return '@';
        case BOTTOM:
            return '+';
        default:
            return ' ';
    }
}

static pixel_t screen_proj(vecf_t cam_space) {
    float f = HEIGHT / (2.0f * tanf(FOV/2.0f));
    float ooz = 1.0f / cam_space.z;

    float screen_x = (WIDTH/2.0f + 2.0f * f * cam_space.x * ooz);
    float screen_y = (HEIGHT/2.0f - f * cam_space.y * ooz);

    return (pixel_t){round(screen_x), round(screen_y), ooz, cam_space};
}

static void buffer_proj(pixel_t p, char c) {
    if (0 <= p.x && p.x < WIDTH && 0 <= p.y && p.y < HEIGHT) {
        int idx = p.y * WIDTH + p.x;
        if (p.ooz >= zbuff[idx]) {
            zbuff[idx] = p.ooz;
            buff[idx] = c;
        }
    }
}

static void draw_line(pixel_t s0, pixel_t s1) {
    int dx = s1.x - s0.x;
    int dy = s1.y - s0.y;
    int steps = MAX(abs(dx), abs(dy));
    if (steps == 0) {
        buffer_proj(s0, '.');
        return;
    }

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;

        pixel_t p = {
            .x = (int)roundf(s0.x + t * dx),
            .y = (int)roundf(s0.y + t * dy),
            .ooz = s0.ooz + t * (s1.ooz - s0.ooz),
            .cam_space = {
                .x = s0.cam_space.x + t * (s1.cam_space.x - s0.cam_space.x),
                .y = s0.cam_space.y + t * (s1.cam_space.y - s0.cam_space.y),
                .z = s0.cam_space.z + t * (s1.cam_space.z - s0.cam_space.z)
            }
        };

        float dist = sqrtf(p.cam_space.x*p.cam_space.x +
                           p.cam_space.y*p.cam_space.y +
                           p.cam_space.z*p.cam_space.z );

        if (dist < 8) {
            buffer_proj(p, '.');
            buffer_proj((pixel_t){p.x+1, p.y, p.ooz, p.cam_space}, '.');
        }
    }
}

static void render_poly(pixel_t p0,  pixel_t p1,  pixel_t p2, float area, face_dir_t dir) {
    int left   = MAX(0,        MIN(p0.x, MIN(p1.x, p2.x)));
    int right  = MIN(WIDTH,  MAX(p0.x, MAX(p1.x, p2.x)));
    int top    = MAX(0,        MIN(p0.y, MIN(p1.y, p2.y)));
    int bottom = MIN(HEIGHT, MAX(p0.y, MAX(p1.y, p2.y)));

    if (fabsf(area) < EPS)
        return;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            pixel_t p = {.x = x, .y = y};

            float w0 = edge(p1, p2, p);
            float w1 = edge(p2, p0, p);
            float w2 = edge(p0, p1, p);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {

                float a = w0 / area;
                float b = w1 / area;
                float c = w2 / area;

                p.ooz = a*p0.ooz + b*p1.ooz + c*p2.ooz;
                p.cam_space = (vecf_t){
                    .x = a*p0.cam_space.x + b*p1.cam_space.x + c*p2.cam_space.x,
                    .z = a*p0.cam_space.y + b*p1.cam_space.y + c*p2.cam_space.y,
                    .y = a*p0.cam_space.z + b*p1.cam_space.z + c*p2.cam_space.z
                };

                // float dist = sqrtf(cam_space.x*cam_space.x +
                                   // cam_space.y*cam_space.y +
                                   // cam_space.z*cam_space.z );

                buffer_proj(p, get_shade_char(dir));
            }
        }
    }
}

static void render_face(camera_t *cam, vecf_t rel, face_t face) {
    pixel_t pixels[4];
    for (int i = 0; i < 4; i++) {
        vecf_t vtx = v_addf(ref_vtxs[face.idxs[i]], rel);
        vtx = v_rotatef(vtx, cam->cost, cam->sint, cam->cosp, cam->sinp);
        if (vtx.z < NEAR_PLANE)
            return;
        pixels[i] = screen_proj(vtx);
    }
    float area1 = edge(pixels[0], pixels[1], pixels[2]);
    if (area1 <= 0) return;
    float area2 = edge(pixels[0], pixels[2], pixels[3]);
    if (area2 <= 0) return;

    render_poly(pixels[0], pixels[1], pixels[2], area1, face.dir);
    render_poly(pixels[0], pixels[2], pixels[3], area2, face.dir);

    }
}

static void render_block(camera_t *cam, veci_t world) {
    vecf_t rel = v_subf(vf(world), cam->pos);

    for (int f = 0; f < 6; f++) {
        render_face(cam, rel, ref_faces[f]);
    }
}

void outline_block(camera_t *cam, veci_t block_pos) {
    vecf_t rel = v_subf(vf(world), cam->pos);
    for (int f = 0; f < 6; f++) {
        face_t face = ref_faces[f];
        vecf_t pixels[4];
        for (int i = 0; i < 4; i++) {
            vecf_t vtx = v_addf(ref_vtxs[face.idxs[i]], rel);
            vtx = v_rotatef(vtx, cam->cost, cam->sint, cam->cosp, cam->sinp);
            if (vtx.z < NEAR_PLANE)
                return;
            pixels[i] = screen_proj(vtx);
        }
        float area1 = edge(pixels[0], pixels[1], pixels[2]);
        if (area1 <= 0) return;
        float area2 = edge(pixels[0], pixels[2], pixels[3]);
        if (area2 <= 0) return;

        for (int i = 0; i < 4; i++) {
            int j = (i+1) % 4;
            pixel_t p0 = pixels[i];
            p0.ooz += 0.01f;
            pixel_t p1 = pixels[j];
            p1.ooz += 0.01f;
            draw_line(p0, p1);
        }
    }
}

static void render_chunk(camera_t *cam, const chunk_t *chunk) {
    veci_t coord = chunk->coord;
    for (int y = 0; y < CHUNK_Y; y++) {
        for (int z = 0; z < CHUNK_Z; z++) {
            for (int x = 0; x < CHUNK_X; x++) {
                if (block_present(chunk, x, y, z)) {
                    veci_t world = {x + coord.x*CHUNK_X,
                                       y + coord.y*CHUNK_Y,
                                       z + coord.z*CHUNK_Z};
                    render_block(cam, world);
                }
            }
        }
    }
}

void render_chunks(camera_t *cam, chunk_t chunks[], int num_chunks) {
    for (int i = 0; i < num_chunks; i++) {
        render_chunk(cam, &chunks[i]);
    }
}

void draw_crosshair(void) {
    pixel_t center = {.x = WIDTH/2, .y = HEIGHT/2, .ooz = INFINITY};
    buffer_proj(center, '+');
    if (WIDTH > 250) {
        pixel_t left = {.x = center.x-1, .y = center.y, .ooz = INFINITY};
        pixel_t right = {.x = center.x+1, .y = center.y, .ooz = INFINITY};
        buffer_proj(left, '<');
        buffer_proj(right, '>');
    }
}
