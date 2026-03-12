#include <signal.h>
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

static chunk_t chunks[8];

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

        float cost, cosp, sint, sinp;
        get_angle(&cost, &cosp, &sint, &sinp, cam.theta, cam.phi);

        move_cam(&cam, cost, cosp, sint, sinp);

        // for (int i = 0; i < 2; i++) {
            // cube_t cube = chunks[i];

            // coord_t projected[8];
            // for (int i = 0; i < 8; i++) {
                // vector_t transformed = v_sub(cube.vtxs[i], cam.pos);
                // vector_t rotated = v_rotate(transformed, cost, cosp, sint, sinp);
                // if (rotated.z > 0.001f)
                    // projected[i] = screen_proj(rotated);
            // }

            // for (int f = 0; f < 6; f++) {
                // face_t face = cube.faces[f];

                // vector_t v0 = cube.vtxs[face.idxs[0]];
                // vector_t v1 = cube.vtxs[face.idxs[1]];
                // vector_t v2 = cube.vtxs[face.idxs[2]];
                // vector_t v3 = cube.vtxs[face.idxs[3]];

                // vector_t face_center = { (v0.x + v1.x + v2.x + v3.x) / 4,
                                         // (v0.y + v1.y + v2.y + v3.y) / 4,
                                         // (v0.z + v1.z + v2.z + v3.z) / 4 };
                // vector_t to_camera = v_sub(cam.pos, face_center);

                // if (v_dot(face.norm, to_camera) > 0) {

                // render_poly(projected[face.idxs[0]],
                            // projected[face.idxs[1]],
                            // projected[face.idxs[2]],
                            // face.c);

                // render_poly(projected[face.idxs[0]],
                            // projected[face.idxs[2]],
                            // projected[face.idxs[3]],
                            // face.c);
                // }
            // }
        // }
        draw_crosshair();
        print_frame();

        usleep(1000000/FPS);
    }

    disable_raw_mode();
    show_cursor();

    free_buffs();

    return 0;
}
