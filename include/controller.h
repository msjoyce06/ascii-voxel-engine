#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "vectors.h"
#include <signal.h>

typedef struct {
    vector_t pos;
    float theta, phi;
    float cost, sint;
    float cosp, sinp;
} camera_t;

extern volatile sig_atomic_t running;

/** Signal Interrupt */
void handle_sigint(int sig);

/** Key Input */
void enable_raw_mode(void);

void disable_raw_mode(void);

/** Update */
void update_cam(camera_t *cam);

#endif
