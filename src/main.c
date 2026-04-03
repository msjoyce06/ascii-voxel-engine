#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "vectors.h"
#include "controller.h"
#include "args.h"
#include "render.h"
#include "world.h"

#define NUM_CHUNKS 8

volatile sig_atomic_t running = 1;
int WIDTH, HEIGHT;
float FOV;
int FPS = 60;

static chunk_t chunks[NUM_CHUNKS];

static camera_t cam = { .pos = {0, 1.75f, 0},
                        .theta = 0, .phi = 0,
                        .cost = 1, .sint = 0,
                        .cosp = 1, .sinp = 0 };

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    enable_raw_mode();
    signal(SIGINT, handle_sigint);

    hide_cursor();
    clear_screen();

    init_buffs();
    load_chunks(chunks, 1);

    // set floor
    for (int x = -16; x < 16; x++) {
        for (int z = -16; z < 16; z++) {
            set_block(chunks, (veci_t){x, -1, z});
        }
    }

    set_block(chunks, (veci_t){-2, 0, 2});
    set_block(chunks, (veci_t){1, 0, 2});
    set_block(chunks, (veci_t){1, 1, 2});

    while (running) {
        set_buffs();

        update(&cam, chunks);

        render_chunks(&cam, chunks, NUM_CHUNKS);
        raycast_block(&cam, chunks);
        outline_block(&cam, (veci_t){-2, 0, 2}, '_');
        outline_block(&cam, (veci_t){1, 0, 2}, '_');
        outline_block(&cam, (veci_t){1, 1, 2}, '_');
        highlight_selection(&cam);
        draw_crosshair();

        print_frame();
        usleep(1000000/FPS);
    }

    disable_raw_mode();
    show_cursor();

    free_buffs();

    return 0;
}
