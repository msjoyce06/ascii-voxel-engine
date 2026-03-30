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

static veci_t get_adjacent_block(camera_t *cam) {
    veci_t block = cam->raycast.block;
    switch (cam->raycast.face) {
        case WEST:
            return (veci_t){block.x-1, block.y, block.z};
        case EAST:
            return (veci_t){block.x+1, block.y, block.z};
        case SOUTH:
            return (veci_t){block.x, block.y, block.z-1};
        case NORTH:
            return (veci_t){block.x, block.y, block.z+1};
        case BOTTOM:
            return (veci_t){block.x, block.y-1, block.z};
        default:
            return (veci_t){block.x, block.y+1, block.z};
    }
}

void update(camera_t *cam, chunk_t chunks[]) {
    int key = read_key();
    if (key != -1) {
        switch (key) {
            // camera
            case 'h':
            case 'j':
            case 'k':
            case 'l':
                if (key == 'h') cam->theta -= 7.5f;
                if (key == 'j') cam->phi -= 4;
                if (key == 'k') cam->phi += 4;
                if (key == 'l') cam->theta += 7.5f;
                update_trig(cam);
                break;

            // movement
            case 'w':
            case 'a':
            case 's':
            case 'd':
                vecf_t forward = {0, 0, 0.15f};
                forward = v_rotatef(forward, cam->cost, -cam->sint, 1, 0);
                vecf_t right = {0.15f, 0, 0};
                right = v_rotatef(right, cam->cost, -cam->sint, 1, 0);
                if (key == 'w') cam->pos = v_addf(cam->pos, forward);
                if (key == 'a') cam->pos = v_subf(cam->pos, right);
                if (key == 's') cam->pos = v_subf(cam->pos, forward);
                if (key == 'd') cam->pos = v_addf(cam->pos, right);
                break;
            case ' ':
                cam->pos = v_addf(cam->pos, (vecf_t){0, 0.15f, 0});
                break;
            case 'z':
            case 'c':
                cam->pos = v_subf(cam->pos, (vecf_t){0, 0.15f, 0});
                break;

            // placing/destroying blocks
            case 'u':
                if (cam->raycast.hit)
                    clear_block(chunks, cam->raycast.block);
                break;
            case 'i':
                if (cam->raycast.hit) {
                    veci_t block = get_adjacent_block(cam);
                    set_block(chunks, block);
                }
                break;
        }
        if (cam->phi > 90.0f) cam->phi = 90.0f;
        if (cam->phi < -90.0f) cam->phi = -90.0f;
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

    if (step_x > 0)
        side_x = ((float)(cell.x + 1) - start.x) * delta_x;
    else if (step_x < 0)
        side_x = (start.x - (float)cell.x) * delta_x;
    else
        side_x = INFINITY;

    if (step_y > 0)
        side_y = ((float)(cell.y + 1) - start.y) * delta_y;
    else if (step_y < 0)
        side_y = (start.y - (float)cell.y) * delta_y;
    else
        side_y = INFINITY;

    if (step_z > 0)
        side_z = ((float)(cell.z + 1) - start.z) * delta_z;
    else if (step_z < 0)
        side_z = (start.z - (float)cell.z) * delta_z;
    else
        side_z = INFINITY;

    float reach = 4.5f;
    for (;;) {
        if (is_solid_block(chunks, cell)) {
            hit = true;
            break;
        }
        if (side_x < side_y && side_x < side_z) {
            if (side_x > reach) break;
            cell.x += step_x;
            side_x += delta_x;
            hit_face = (step_x > 0) ? WEST : EAST;
        }
        else if (side_y < side_z) {
            if (side_y > reach) break;
            cell.y += step_y;
            side_y += delta_y;
            hit_face = (step_y > 0) ? BOTTOM : TOP;
        }
        else {
            if (side_z > reach) break;
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
