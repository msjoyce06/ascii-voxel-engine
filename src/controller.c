#include "controller.h"
#include <termios.h>
#include <unistd.h>
#include <math.h>

static struct termios oldt;

/** Signal Interrupt */
void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

/** Key Input */
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

/** Update */
static void update_trig(camera_t *cam) {
    float theta_r = cam->theta * M_PI / 180;
    float phi_r = cam->phi * M_PI / 180;

    cam->cost = cosf(theta_r);
    cam->sint = sinf(theta_r);
    cam->cosp = cosf(phi_r);
    cam->sinp = sinf(phi_r);
}

static void get_axes(const camera_t *cam, vector_t *x_axis, vector_t *z_axis) {
    *x_axis = v_rotate(x_axis, cam->cost, 1, -(cam->sint), 0);
    *z_axis = v_rotate(z_axis, cam->cost, 1, -(cam->sint), 0);
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
                vector_t x_axis = {0.12f, 0, 0};
                vector_t z_axis = {0, 0, 0.12f};
                get_axes(cam, &x_axis, &z_axis);
                if (key == 'w') cam->pos = v_add(cam->pos, z_axis);
                if (key == 'a') cam->pos = v_sub(cam->pos, x_axis);
                if (key == 's') cam->pos = v_sub(cam->pos, z_axis);
                if (key == 'd') cam->pos = v_add(cam->pos, x_axis);
                break;
            }
        }
        if (cam->phi > 89.0f) cam->phi = 89.0f;
        if (cam->phi < -89.0f) cam->phi = -89.0f;
    }
}
