#include "bitwave.h"
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(unsigned ms) { Sleep(ms); }
#else
static void sleep_ms(unsigned ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
#endif

int main(void) {
    if (!BW_init()) {
        fprintf(stderr, "Failed to initialize\n");
        return 1;
    }

    printf("Playing sound...\n");
    /* Since we copy the file, it is in the same directory as the executable */
    if (!BW_play("DL3PDAlois02.mp3")) {
        fprintf(stderr, "Failed to play sound\n");
    }

    sleep_ms(25000); /* Adjust time to length of audio */
    BW_close();
    return 0;
}
