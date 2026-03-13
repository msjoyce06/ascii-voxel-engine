#include <signal.h>
#include <unistd.h>

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

static camera_t cam = { .pos = {0, 1.75f, 0}, .theta = 0, .phi = 0 };

int main(int argc, char *argv[]) {
    enable_raw_mode();
    signal(SIGINT, handle_sigint);

    parse_args(argc, argv);

    hide_cursor();
    clear_screen();

    init_buffs();
    load_chunks(chunks, 1);

    while (running) {
        set_buffs();

        update_cam(&cam);

        render_chunks(&cam, chunks, NUM_CHUNKS);
        draw_crosshair();

        print_frame();
        usleep(1000000/FPS);
    }

    disable_raw_mode();
    show_cursor();

    free_buffs();

    return 0;
}
