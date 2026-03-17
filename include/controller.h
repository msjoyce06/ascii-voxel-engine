#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "vectors.h"
#include "world.h"
#include <signal.h>
#include <stdbool.h>

typedef struct {
    bool hit;
    veci_t block;
    face_dir_t face;
} ray_hit_t;

typedef struct {
    vecf_t pos;
    float theta, phi;
    float cost, sint;
    float cosp, sinp;
    veci_t looking_at;
} camera_t;

extern volatile sig_atomic_t running;

/** signal interrupt */
void handle_sigint(int sig);

/** key input */
void enable_raw_mode(void);
void disable_raw_mode(void);

/** update */
void update_cam(camera_t *cam);
void raycast_block(camera_t *cam, chunk_t chunks[]);

#endif
