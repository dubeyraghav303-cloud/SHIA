#include "signal_handler.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void shcr_generic_handler(int signo) {
    printf("[SHCR-Internal] Caught signal %d inside process\n", signo);
    // In a full implementation, this would communicate with the runtime guard
    exit(signo);
}

void shcr_install_handlers() {
    signal(SIGSEGV, shcr_generic_handler);
    signal(SIGFPE, shcr_generic_handler);
    signal(SIGILL, shcr_generic_handler);
}
