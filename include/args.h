#ifndef ARGS_H
#define ARGS_H

extern int WIDTH, HEIGHT;
extern float FOV;
extern int FPS;

void err_print_usage(void);

int parse_int(const char *str);

void parse_args(int argc, char *argv[]);

#endif
