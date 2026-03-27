#include "render.h"
#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
    float x;
    float y;
    float ooz;
} coord_t;

typedef struct {
    int idxs[4];
    vector_t norm;
} face_t;

static char *buff;
static float *zbuff;
static int bufflen;

const float NEAR = 0.001f;
const float EDGE_WIDTH = 0.025f;

static vector_t ref_vtxs[8] = {
    {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
};

static face_t ref_faces[6] = {
    { .idxs = {4, 0, 3, 7}, .norm = {-1,  0,  0} }, // x = 0
    { .idxs = {1, 5, 6, 2}, .norm = { 1,  0,  0} }, // x = 1
    { .idxs = {0, 1, 2, 3}, .norm = { 0,  0, -1} }, // z = 0
    { .idxs = {5, 4, 7, 6}, .norm = { 0,  0,  1} }, // z = 1
    { .idxs = {4, 5, 2, 1}, .norm = { 0, -1,  0} }, // y = 0
    { .idxs = {3, 2, 6, 7}, .norm = { 0,  1,  0} }  // y = 1
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
static float edge(coord_t v1, coord_t v2, coord_t p) {
    return (p.x - v1.x) * (v2.y - v1.y) - (p.y - v1.y) * (v2.x - v1.x);
}

static int should_draw_edge(coord_t s0, coord_t s1, vector_t v0, vector_t v1) {
    float dx = fabsf(s1.x - s0.x);
    float dy = fabsf(s1.y - s0.y);
    float dist = sqrtf(v0.x*v1.x + v0.y*v1.y + v0.z*v1.z);

    if (dist > 8.0f && dy < 0.1f*dx)
        return 0;
    return 1;
}

static char get_shade_char(vector_t norm, float dist, int is_edge) {
    int levels;
    char *ramp;
    if (WIDTH < 250) {
        ramp = " .+?l$%@";
        levels = 8;
    } else {
        ramp = " .:+!?ilJ$KX%0#@";
        levels = 16;
    }
    float base_shade;

    if (norm.y == 1.0f)       // top
        base_shade = 1.1f;
    else if (norm.z == -1.0f) // south
        base_shade = 0.8f;
    else if (norm.x == -1.0f) // west
        base_shade = 0.65f;
    else if (norm.z == 1.0f)  // north
        base_shade = 0.55f;
    else if (norm.x == 1.0f)  // east
        base_shade = 0.45f;
    else                      // bottom
        base_shade = 0.25f;

    float near = 0.5f;
    float far = 18.0f;
    float d = 1.0f - (dist - near) / (far - near);
    if (d < 0.0f) d = 0.0f;
    if (d > 1.0f) d = 1.0f;

    float brightness = base_shade * d;
    int idx = (int)(brightness * (levels - 1) + 0.5f);
    if (idx < 0) idx = 0;
    if (idx > levels - 1) idx = levels - 1;
    if (idx == 0 && is_edge) idx = 1;

    return ramp[idx];
}

static coord_t screen_proj(vector_t cam_space) {
    float f = HEIGHT / (2.0f * tanf(FOV/2.0f));
    float ooz = 1.0f / cam_space.z;

    float screen_x = (WIDTH/2.0f + 2.0f * f * cam_space.x * ooz);
    float screen_y = (HEIGHT/2.0f - f * cam_space.y * ooz);

    return (coord_t){screen_x, screen_y, ooz};
}

static void buffer_proj(coord_t p, char c) {
    if (0 <= p.x && p.x < WIDTH && 0 <= p.y && p.y < HEIGHT) {
        int idx = (int)p.y * WIDTH + (int)p.x;
        if (p.ooz > zbuff[idx]) {
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

static void outline_block(camera_t *cam, veci_t block_pos) {
    vecf_t rel = v_subf(vf(block_pos), cam->pos);
    for (int f = 0; f < 6; f++) {
        face_t face = ref_faces[f];
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

static void render_poly(coord_t s0,  coord_t s1,  coord_t s2, int ignore_edge,
                        vector_t v0, vector_t v1, vector_t v2, vector_t norm) {
    int left   = MAX(0,      (int)floorf(MIN(s0.x, MIN(s1.x, s2.x))));
    int right  = MIN(WIDTH,  (int)ceilf(MAX(s0.x, MAX(s1.x, s2.x))));
    int top    = MAX(0,      (int)floorf(MIN(s0.y, MIN(s1.y, s2.y))));
    int bottom = MIN(HEIGHT, (int)ceilf(MAX(s0.y, MAX(s1.y, s2.y))));

    float area = edge(s0, s1, s2);
    const float eps = 1e-6f;
    if (fabsf(area) < eps)
        return;

    int draw_e0 = should_draw_edge(s1, s2, v1, v2) && (ignore_edge != 0);
    int draw_e1 = should_draw_edge(s2, s0, v2, v0) && (ignore_edge != 2);
    int draw_e2 = should_draw_edge(s0, s1, v0, v1) && (ignore_edge != 1);

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            coord_t p = {x + 0.5f, y + 0.5f, 0};

            float w0 = edge(s1, s2, p);
            float w1 = edge(s2, s0, p);
            float w2 = edge(s0, s1, p);

            if ((w0 >= -eps && w1 >= -eps && w2 >= -eps) ||
                (w0 <= eps && w1 <= eps && w2 <= eps)) {

                float a = w0 / area;
                float b = w1 / area;
                float c = w2 / area;

                p.ooz = a*s0.ooz + b*s1.ooz + c*s2.ooz;

                int on_edge = ((draw_e0 && fabsf(a) < EDGE_WIDTH) ||
                               (draw_e1 && fabsf(b) < EDGE_WIDTH) ||
                               (draw_e2 && fabsf(c) < EDGE_WIDTH));

                vector_t cam_space = { a*v0.x + b*v1.x + c*v2.x,
                                       a*v0.y + b*v1.y + c*v2.y,
                                       a*v0.z + b*v1.z + c*v2.z };
                float dist = sqrtf(cam_space.x*cam_space.x +
                                   cam_space.y*cam_space.y +
                                   cam_space.z*cam_space.z );

                if (on_edge && dist < 15)
                    buffer_proj(p, get_shade_char(norm, dist+8, 1));
                else
                    buffer_proj(p, get_shade_char(norm, dist, 0));
            }
        }
    }
}

static void render_face(camera_t *cam, vector_t rel, face_t face) {
    // backface culling
    vector_t v = v_add(ref_vtxs[face.idxs[0]], rel);
    if (v_dot(face.norm, v) >= 0)
        return;

    vector_t cam_space[4];
    coord_t projected[4];
    for (int i = 0; i < 4; i++) {
        vector_t vtx = v_add(ref_vtxs[face.idxs[i]], rel);
        cam_space[i] = v_rotate(vtx, cam->cost, cam->sint, cam->cosp, cam->sinp);
        if (cam_space[i].z < NEAR)
            return;
        projected[i] = screen_proj(cam_space[i]);
    }
    render_poly(projected[0], projected[1], projected[2], 2,
                cam_space[0], cam_space[1], cam_space[2], face.norm);
    render_poly(projected[0], projected[2], projected[3], 1,
                cam_space[0], cam_space[2], cam_space[3], face.norm);
}

static void render_block(camera_t *cam, vector_t world) {
    vector_t rel = v_sub(world, cam->pos);

    for (int f = 0; f < 6; f++) {
        render_face(cam, rel, ref_faces[f]);
    }
}

static void render_chunk(camera_t *cam, const chunk_t *chunk) {
    vector_t coord = chunk->coord;
    for (int y = 0; y < CHUNK_Y; y++) {
        for (int z = 0; z < CHUNK_Z; z++) {
            for (int x = 0; x < CHUNK_X; x++) {
                if (block_present(chunk, x, y, z)) {
                    vector_t world = {x + coord.x*CHUNK_X,
                                      y + coord.y*CHUNK_Y,
                                      z + coord.z*CHUNK_Z};
                    render_block(cam, world);
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
    coord_t center = {WIDTH/2, HEIGHT/2, INFINITY};
    buffer_proj(center, '+');
    if (WIDTH > 250) {
        coord_t left = {center.x-1, center.y, INFINITY};
        coord_t right = {center.x+1, center.y, INFINITY};
        buffer_proj(left, '<');
        buffer_proj(right, '>');
    }
}

