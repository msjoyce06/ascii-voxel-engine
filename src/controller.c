#include "controller.h"
#include <termios.h>
#include <unistd.h>

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

int read_key(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return (unsigned char)c;
    return -1;
}

