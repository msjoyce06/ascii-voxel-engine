#include "controller.h"
#include "render.h"
#include <termios.h>
#include <unistd.h>
#include <math.h>

static struct termios oldt;

/** signal interrupt */
void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

/** key input */
void enable_raw_mode(void) {
    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static int read_key(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return (unsigned char)c;
    return -1;
}

/** update */
static void update_trig(camera_t *cam) {
    float theta_r = cam->theta * M_PI / 180;
    float phi_r = cam->phi * M_PI / 180;

    cam->cost = cosf(theta_r);
    cam->sint = sinf(theta_r);
    cam->cosp = cosf(phi_r);
    cam->sinp = sinf(phi_r);
}

void update_cam(camera_t *cam) {
    int key = read_key();
    if (key != -1) {
        switch (key) {
            case 'h':
            case 'j':
            case 'k':
            case 'l': {
                if (key == 'h') cam->theta -= 5;
                if (key == 'j') cam->phi -= 5;
                if (key == 'k') cam->phi += 5;
                if (key == 'l') cam->theta += 5;
                update_trig(cam);
                break;
            }
            case 'w':
            case 'a':
            case 's':
            case 'd': {
                vecf_t forward = {0, 0, 0.12f};
                forward = v_rotatef(forward, cam->cost, -cam->sint, 1, 0);
                vecf_t right = {0.12f, 0, 0};
                right = v_rotatef(right, cam->cost, -cam->sint, 1, 0);
                if (key == 'w') cam->pos = v_addf(cam->pos, forward);
                if (key == 'a') cam->pos = v_subf(cam->pos, right);
                if (key == 's') cam->pos = v_subf(cam->pos, forward);
                if (key == 'd') cam->pos = v_addf(cam->pos, right);
                break;
            }
        }
        if (cam->phi > 89.0f) cam->phi = 89.0f;
        if (cam->phi < -89.0f) cam->phi = -89.0f;
    }
}

void raycast_block(camera_t *cam, chunk_t chunks[]) {
    vecf_t ray = cam->pos;

    vecf_t dir = {cam->sint*cam->cosp, cam->sinp, cam->cost*cam->cosp};

    vecf_t step = (vecf_t){
        sqrtf(1 + (dir.z/dir.x)*(dir.z/dir.x)), sqrt(1 +
    }

    vecf_t end = (vecf_t){
        start.x + cam->sint*cam->cosp
}

// void raycast_block(camera_t *cam, chunk_t chunks[]) {
    // vecf_t dir = {
        // .x = cam->sint*cam->cosp,
        // .y = cam->sinp,
        // .z = cam->cost*cam->cosp
    // };
    // vecf_t ray = cam->pos;
    // for (int i = 0; i < 5; i++) {
        // if (block_present_world(ray, chunks)) {
            // cam->raycast.hit = true;
            // cam->raycast.block = (veci_t){(int)floorf(ray.x),
                                          // (int)floorf(ray.y),
                                          // (int)floorf(ray.z)};
        // }
        // ray = v_addf(ray, dir);
    // }
// }
