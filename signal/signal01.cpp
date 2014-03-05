#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "signal01.h"

void sig_usr(int signo) {
    if (SIGUSR1 == signo) {
        printf("received SIGUSR1 singal.\n");
    }
    else if (SIGUSR2 == signo) {
        printf("received SIGUSR2 singal.\n");
    }
    else {
        printf("received unknown singal[%d].\n", signo);
    }
}

int main() {
    if (SIG_ERR == signal(SIGUSR1, sig_usr)) {
        printf("can't catch SIGUSR1 singal.\n");
    }
    if (SIG_ERR == signal(SIGUSR2, sig_usr)) {
        printf("can't catch SIGUSR2 singal.\n");
    }

    for (;;) {
        pause();
    }
}
