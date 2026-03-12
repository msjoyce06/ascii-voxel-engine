#include <signal.h>
#include <math.h>
#include <unistd.h>

#include "vectors.h"
#include "controller.h"
#include "args.h"
#include "render.h"
#include "world.h"

volatile sig_atomic_t running = 1;
int WIDTH, HEIGHT;
float FOV;
int FPS = 60;

static chunk_t *chunk;

static camera_t cam = { .pos = {0, 1.75f, 0}, .theta = 0, .phi = 0 };

int main(int argc, char *argv[]) {
    enable_raw_mode();
    signal(SIGINT, handle_sigint);

    hide_cursor();
    clear_screen();

    parse_args(argc, argv);

    init_buffs();

    while (running) {
        set_buffs();

        float theta_r = cam.theta * M_PI / 180;
        float phi_r = cam.phi * M_PI / 180;
        float cost = cosf(theta_r);
        float cosp = cosf(phi_r);
        float sint = sinf(theta_r);
        float sinp = sinf(phi_r);

        int key = read_key();
        if (key != -1) {
            vector_t x_axis = (vector_t){0.12f, 0, 0};
            vector_t z_axis = (vector_t){0, 0, 0.12f};
            x_axis = v_rotate(x_axis, cost, 1, -sint, 0);
            z_axis = v_rotate(z_axis, cost, 1, -sint, 0);
            if (key == 'w')
                cam.pos = v_add(cam.pos, z_axis);
            if (key == 'a')
                cam.pos = v_sub(cam.pos, x_axis);
            if (key == 's')
                cam.pos = v_sub(cam.pos, z_axis);
            if (key == 'd')
                cam.pos = v_add(cam.pos, x_axis);
            if (key == 'h')
                cam.theta -= 5;
            if (key == 'j')
                cam.phi -= 5;
            if (key == 'k')
                cam.phi += 5;
            if (key == 'l')
                cam.theta += 5;

            if (cam.phi > 89.0f) cam.phi = 89.0f;
            if (cam.phi < -89.0f) cam.phi = -89.0f;
        }



        for (int i = 0; i < 2; i++) {
            cube_t cube = chunks[i];

            coord_t projected[8];
            for (int i = 0; i < 8; i++) {
                vector_t transformed = v_sub(cube.vtxs[i], cam.pos);
                vector_t rotated = v_rotate(transformed, cost, cosp, sint, sinp);
                if (rotated.z > 0.001f)
                    projected[i] = screen_proj(rotated);
            }

            for (int f = 0; f < 6; f++) {
                face_t face = cube.faces[f];

                vector_t v0 = cube.vtxs[face.idxs[0]];
                vector_t v1 = cube.vtxs[face.idxs[1]];
                vector_t v2 = cube.vtxs[face.idxs[2]];
                vector_t v3 = cube.vtxs[face.idxs[3]];

                vector_t face_center = { (v0.x + v1.x + v2.x + v3.x) / 4,
                                         (v0.y + v1.y + v2.y + v3.y) / 4,
                                         (v0.z + v1.z + v2.z + v3.z) / 4 };
                vector_t to_camera = v_sub(cam.pos, face_center);

                if (v_dot(face.norm, to_camera) > 0) {

                render_poly(projected[face.idxs[0]],
                            projected[face.idxs[1]],
                            projected[face.idxs[2]],
                            face.c);

                render_poly(projected[face.idxs[0]],
                            projected[face.idxs[2]],
                            projected[face.idxs[3]],
                            face.c);
                }
            }
        }
        draw_crosshair();
        print_frame();

        usleep(1000000/FPS);
    }

    disable_raw_mode();
    show_cursor();

    free_buffs();

    return 0;
}
