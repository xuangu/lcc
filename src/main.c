#include <stdio.h>

static int err_cnt = 0;
static int err_limit = 5;

void error(const char *fmt, ...) {

    if (err_cnt++ >= err_limit) {
        err_cnt = -1;
        error("too many errors\n");
        printf("going exit\n");
        fflush(stdout);
        exit(1);
    }

    printf("error 1\n");
}

int main() {

    for (int i = 0; i <= 5; i++) {
        error("hello\n");
    }

    // error("test error");

    return 0;
}