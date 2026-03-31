#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "vectors.h"
#include "world.h"
#include <signal.h>
#include <stdbool.h>

typedef enum {
    WEST,
    EAST,
    SOUTH,
    NORTH,
    BOTTOM,
    TOP
} face_dir_t;

typedef struct {
    bool hit;
    veci_t block;
    face_dir_t face;
} ray_hit_t;

typedef struct camera {
    vecf_t pos;
    float theta, phi;
    float cost, sint;
    float cosp, sinp;
    ray_hit_t raycast;
} camera_t;

/** signal interrupt */
void handle_sigint(int sig);

/** key input */
void enable_raw_mode(void);
void disable_raw_mode(void);

/** helpers */
veci_t get_adjacent_block(veci_t block, face_dir_t dir);

/** update */
void update(camera_t *cam, chunk_t chunks[]);
void raycast_block(camera_t *cam, const chunk_t chunks[]);

#endif
