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
    float x;
    float y;
    float ooz;
    vecf_t cam_space;
} screen_vtx_t;

typedef struct {
    int idxs[4];
    face_dir_t dir;
} face_t;

static char *buff;
static float *zbuff;
static int bufflen;

const float EPS = 1e-8f;
const float OUTLINE_PAD = 0.003f;
const float NEAR_PLANE = 1e-6f;

static vecf_t ref_vtxs[8] = {
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f}
};

static face_t ref_faces[6] = {
    { .idxs = {4, 0, 3, 7}, .dir = WEST },   // x = 0
    { .idxs = {1, 5, 6, 2}, .dir = EAST },   // x = 1
    { .idxs = {0, 1, 2, 3}, .dir = SOUTH },  // z = 0
    { .idxs = {5, 4, 7, 6}, .dir = NORTH },  // z = 1
    { .idxs = {4, 5, 1, 0}, .dir = BOTTOM }, // y = 0
    { .idxs = {3, 2, 6, 7}, .dir = TOP }     // y = 1
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
static char get_shade_char(face_dir_t face, float dist) {
    char *ramp;
    int levels;
    if (WIDTH < 250) {
        ramp = " .+?l$%@";
        levels = 8;
    } else {
        ramp = " .:+!?ilJ$KX%0#@";
        levels = 16;
    }
    float base_shade;

    if (face == TOP)
        base_shade = 1.0f;
    else if (face == SOUTH)
        base_shade = 0.6f;
    else if (face == WEST)
        base_shade = 0.5f;
    else if (face == NORTH)
        base_shade = 0.6f;
    else if (face == EAST)
        base_shade = 0.5f;
    else  // face ==  BOTTOM
        base_shade = 0.3f;

    float near = 0.5f;
    float far = 18.0f;
    float d = 1.0f - (dist - near) / (far - near);
    if (d < 0.0f) d = 0.0f;
    if (d > 1.0f) d = 1.0f;

    float brightness = base_shade * d;
    int idx = (int)(brightness * (levels - 1) + 0.5f);
    if (idx < 0) idx = 0;
    if (idx > levels - 1) idx = levels - 1;

    return ramp[idx];
}

static screen_vtx_t screen_proj(vecf_t cam_space) {
    float f = HEIGHT / (2.0f * tanf(FOV/2.0f));
    float ooz = 1.0f / cam_space.z;

    float screen_x = WIDTH/2.0f + 2.0f * f * cam_space.x * ooz;
    float screen_y = HEIGHT/2.0f - f * cam_space.y * ooz;

    return (screen_vtx_t){screen_x, screen_y, ooz, cam_space};
}

static void buffer_proj(screen_vtx_t p, char c) {
    if (0 <= p.x && p.x < WIDTH && 0 <= p.y && p.y < HEIGHT) {
        int idx = (int)p.y * WIDTH + (int)p.x;
        if (p.ooz > zbuff[idx]) {
            zbuff[idx] = p.ooz;
            buff[idx] = c;
        }
    }
}

static void draw_line(screen_vtx_t s0, screen_vtx_t s1) {
    float dx = s1.x - s0.x;
    float dy = s1.y - s0.y;
    int steps = (int)ceilf(fmaxf(fabsf(dx), fabsf(dy)));
    if (steps == 0) {
        buffer_proj(s0, '.');
        return;
    }

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;

        screen_vtx_t p = {
            .x = s0.x + t * dx,
            .y = s0.y + t * dy,
            .ooz = s0.ooz + t * (s1.ooz - s0.ooz) + 0.01f,
            .cam_space = {
                .x = s0.cam_space.x + t * (s1.cam_space.x - s0.cam_space.x),
                .y = s0.cam_space.y + t * (s1.cam_space.y - s0.cam_space.y),
                .z = s0.cam_space.z + t * (s1.cam_space.z - s0.cam_space.z)
            }
        };

        buffer_proj(p, '.');
        if (WIDTH > 250)
            buffer_proj((screen_vtx_t){p.x+1, p.y, p.ooz, p.cam_space}, '.');
    }
}

static bool is_top_left(screen_vtx_t v1, screen_vtx_t v2) {
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    return (dy > EPS) || (fabsf(dy) <= EPS && dx < -EPS);
}

static inline float edge(screen_vtx_t v1, screen_vtx_t v2, screen_vtx_t p) {
    return (p.x - v1.x)*(v2.y - v1.y) - (p.y - v1.y)*(v2.x - v1.x);
}

static inline float snap_to_zero(float w) {
    return (fabsf(w) < EPS) ? 0 : w;
}

void outline_block(camera_t *cam, veci_t block_pos) {

    screen_vtx_t screen_vtxs[8];
    for (int v = 0; v < 8; v++) {
        vecf_t block_pos_vtx = v_addf(vf(block_pos), ref_vtxs[v]);
        vecf_t cam_vtx = v_subf(block_pos_vtx, cam->pos);
        cam_vtx = v_rotatef(cam_vtx, cam->cost, cam->sint,
                                     cam->cosp, cam->sinp);
        if (cam_vtx.z < NEAR_PLANE)
            return;

        screen_vtxs[v] = screen_proj(cam_vtx);
    }

    // front face
    draw_line(screen_vtxs[0], screen_vtxs[1]);
    draw_line(screen_vtxs[1], screen_vtxs[2]);
    draw_line(screen_vtxs[2], screen_vtxs[3]);
    draw_line(screen_vtxs[3], screen_vtxs[0]);
    // back face
    draw_line(screen_vtxs[4], screen_vtxs[5]);
    draw_line(screen_vtxs[5], screen_vtxs[6]);
    draw_line(screen_vtxs[6], screen_vtxs[7]);
    draw_line(screen_vtxs[7], screen_vtxs[4]);
    // connecting edges
    draw_line(screen_vtxs[0], screen_vtxs[4]);
    draw_line(screen_vtxs[1], screen_vtxs[5]);
    draw_line(screen_vtxs[2], screen_vtxs[6]);
    draw_line(screen_vtxs[3], screen_vtxs[7]);
}

static void render_poly(screen_vtx_t p0,  screen_vtx_t p1,  screen_vtx_t p2,
                        float area, face_dir_t dir) {
    int left   = MAX(0, (int)floorf(fminf(p0.x, fminf(p1.x, p2.x))));
    int right  = MIN(WIDTH, (int)ceilf(fmaxf(p0.x, fmaxf(p1.x, p2.x))));
    int top    = MAX(0, (int)floorf(fminf(p0.y, fminf(p1.y, p2.y))));
    int bottom = MIN(HEIGHT, (int)ceilf(fmaxf(p0.y, fmaxf(p1.y, p2.y))));

    if (fabsf(area) < EPS)
        return;

    bool tl0 = is_top_left(p1, p2);
    bool tl1 = is_top_left(p2, p0);
    bool tl2 = is_top_left(p0, p1);

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            screen_vtx_t p = {.x = x + 0.5f, .y = y + 0.5f};

            float w0 = snap_to_zero(edge(p1, p2, p));
            float w1 = snap_to_zero(edge(p2, p0, p));
            float w2 = snap_to_zero(edge(p0, p1, p));

            if ((w0 > 0 || (fabsf(w0) == 0 && tl0)) &&
                (w1 > 0 || (fabsf(w1) == 0 && tl1)) &&
                (w2 > 0 || (fabsf(w2) == 0 && tl2))) {

                float a = w0 / area;
                float b = w1 / area;
                float c = w2 / area;

                p.ooz = a*p0.ooz + b*p1.ooz + c*p2.ooz;
                p.cam_space = (vecf_t){
                    .x = a*p0.cam_space.x + b*p1.cam_space.x + c*p2.cam_space.x,
                    .y = a*p0.cam_space.y + b*p1.cam_space.y + c*p2.cam_space.y,
                    .z = a*p0.cam_space.z + b*p1.cam_space.z + c*p2.cam_space.z
                };

                float dist = sqrtf(p.cam_space.x * p.cam_space.x +
                                   p.cam_space.y * p.cam_space.y +
                                   p.cam_space.z * p.cam_space.z );

                buffer_proj(p, get_shade_char(dir, dist));
            }
        }
    }
}

static void render_face(camera_t *cam, veci_t block_pos, face_t face) {

    screen_vtx_t pixels[4];
    for (int i = 0; i < 4; i++) {
        vecf_t block_pos_vtx = v_addf(vf(block_pos), ref_vtxs[face.idxs[i]]);
        vecf_t cam_vtx = v_subf(block_pos_vtx, cam->pos);
        cam_vtx = v_rotatef(cam_vtx, cam->cost, cam->sint,
                                     cam->cosp, cam->sinp);

        if (cam_vtx.z < NEAR_PLANE)
            return;

        pixels[i] = screen_proj(cam_vtx);
    }

    float area1 = edge(pixels[0], pixels[1], pixels[2]);
    if (area1 <= 0)
        return;

    float area2 = edge(pixels[0], pixels[2], pixels[3]);
    if (area2 <= 0)
        return;

    render_poly(pixels[0], pixels[1], pixels[2], area1, face.dir);
    render_poly(pixels[0], pixels[2], pixels[3], area2, face.dir);
}

static void render_block(camera_t *cam, veci_t block_pos) {

    for (int f = 0; f < 6; f++) {
        render_face(cam, block_pos, ref_faces[f]);
    }
}

static void render_chunk(camera_t *cam, const chunk_t *chunk) {
    veci_t coord = chunk->coord;
    for (int y = 0; y < CHUNK_Y; y++) {
        for (int z = 0; z < CHUNK_Z; z++) {
            for (int x = 0; x < CHUNK_X; x++) {
                if (is_solid_in_chunk(chunk, (veci_t){x, y, z})) {
                    veci_t block_pos = {x + coord.x*CHUNK_X,
                                    y + coord.y*CHUNK_Y,
                                    z + coord.z*CHUNK_Z};
                    render_block(cam, block_pos);
                }
            }
        }
    }
}

void render_chunks(camera_t *cam, const chunk_t chunks[], int num_chunks) {
    for (int i = 0; i < num_chunks; i++) {
        render_chunk(cam, &chunks[i]);
    }
}

void highlight_selection(camera_t *cam) {
    if (cam->raycast.hit) {
        veci_t block = cam->raycast.block;
        outline_block(cam, block);
    }
}

void draw_crosshair(void) {
    screen_vtx_t center = {.x = WIDTH/2, .y = HEIGHT/2, .ooz = INFINITY};
    buffer_proj(center, '+');
    if (WIDTH > 250) {
        screen_vtx_t left = {.x = center.x-1, .y = center.y, .ooz = INFINITY};
        screen_vtx_t right = {.x = center.x+1, .y = center.y, .ooz = INFINITY};

        buffer_proj(left, '<');
        buffer_proj(right, '>');
    }
}
