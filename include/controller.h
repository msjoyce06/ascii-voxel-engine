#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "vectors.h"
#include <signal.h>

typedef struct {
    vector_t pos;
    float theta;
    float phi;
} camera_t;

extern volatile sig_atomic_t running;

/** signal interrupt */
void handle_sigint(int sig);

/** key input */
void enable_raw_mode(void);

void disable_raw_mode(void);

int read_key(void);

#endif
