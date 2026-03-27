#include "controller.h"
#include "render.h"
#include <termios.h>
#include <unistd.h>
#include <math.h>

static struct termios oldt;
extern volatile sig_atomic_t running;

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

void raycast_block(camera_t *cam, const chunk_t chunks[]) {
    vecf_t start = cam->pos;

    vecf_t dir = {
        .x = cam->sint * cam->cosp,
        .y = cam->sinp,
        .z = cam->cost * cam->cosp
    };

    veci_t cell = {
        .x = (int)floorf(cam->pos.x),
        .y = (int)floorf(cam->pos.y),
        .z = (int)floorf(cam->pos.z),
    };

    int step_x = (dir.x > 0) ? 1 : (dir.x < 0) ? -1 : 0;
    int step_y = (dir.y > 0) ? 1 : (dir.y < 0) ? -1 : 0;
    int step_z = (dir.z > 0) ? 1 : (dir.z < 0) ? -1 : 0;

    float delta_x = (dir.x != 0.0f) ? fabsf(1.0f / dir.x) : INFINITY;
    float delta_y = (dir.y != 0.0f) ? fabsf(1.0f / dir.y) : INFINITY;
    float delta_z = (dir.z != 0.0f) ? fabsf(1.0f / dir.z) : INFINITY;

    float side_x, side_y, side_z;
    face_dir_t hit_face = NORTH; // placeholder
    bool hit = false;

    if (dir.x > 0)
        side_x = ((float)(cell.x + 1) - start.x) * delta_x;
    else
        side_x = (start.x - (float)cell.x) * delta_x;

    if (dir.y > 0)
        side_y = ((float)(cell.y + 1) - start.y) * delta_y;
    else
        side_y = (start.y - (float)cell.y) * delta_y;

    if (dir.z > 0)
        side_z = ((float)(cell.z + 1) - start.z) * delta_z;
    else
        side_z = (start.z - (float)cell.z) * delta_z;

    for (int i = 0; i < 5; i++) {
        if (is_solid_block(chunks, cell)) {
            hit = true;
            break;
        }

        if (side_x < side_y && side_x < side_z) {
            cell.x += step_x;
            side_x += delta_x;
            hit_face = (step_x > 0) ? WEST : EAST;
        }
        else if (side_y < side_z) {
            cell.y += step_y;
            side_y += delta_y;
            hit_face = (step_y > 0) ? BOTTOM : TOP;
        }
        else {
            cell.z += step_z;
            side_z += delta_z;
            hit_face = (step_z > 0) ? SOUTH : NORTH;
        }
    }
    cam->raycast.hit = hit;
    if (hit) {
        cam->raycast.block = cell;
        cam->raycast.face = hit_face;
    }
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
