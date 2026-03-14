#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

void err_print_usage(void) {
    fprintf(stderr, "Usage: ./render -w <width> -h <height> [-r <resolution] [--fov <fov>] [--fps <fps>]\n");
}

int parse_int(const char *str) {
    char *endptr;
    long val = strtol(str, &endptr, 10);

    if ((endptr == str) || (*endptr != '\0')) {
        fprintf(stderr, "Error: %s is not a number\n", str);
        err_print_usage();
        exit(1);
    }
    if (val <= 0) {
        fprintf(stderr, "Error: arguments must be positive\n");
        err_print_usage();
        exit(1);
    }
    if (val > INT_MAX) {
        fprintf(stderr, "Error: number too large\n");
        exit(1);
    }
    return (int)val;
}

void parse_args(int argc, char *argv[]) {

    int w_flag = 0, h_flag = 0;
    int fov_deg = 80;

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (strcmp(arg, "-w") == 0) {
            if (i + 1 >= argc) {
                err_print_usage();
                exit(1);
            }
            w_flag++;
            WIDTH = parse_int(argv[++i]);
        }
        else if (strcmp(arg, "-h") == 0) {
            if (i + 1 >= argc) {
                err_print_usage();
                exit(1);
            }
            h_flag++;
            HEIGHT = parse_int(argv[++i]);
        }
        else if (strcmp(arg, "--fov") == 0) {
            if (i + 1 >= argc) {
                err_print_usage();
                exit(1);
            }
            fov_deg = parse_int(argv[++i]);
        }
        else if (strcmp(arg, "--fps") == 0) {
            if (i + 1 >= argc) {
                err_print_usage();
                exit(1);
            }
            FPS = parse_int(argv[++i]);
        }
        else {
            err_print_usage();
            exit(1);
        }
    }
    if (w_flag != 1 || h_flag != 1) {
        err_print_usage();
        exit(1);
    }
    FOV = fov_deg * M_PI / 180;
}
