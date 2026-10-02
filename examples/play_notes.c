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

    BW_Sound* melody[] = {
        BW_create_sound_note(BW_E, 4, 0.250),
        BW_create_sound_note(BW_G, 4, 0.250),
        BW_create_sound_note(BW_B, 4, 0.250),
        BW_create_sound_note(BW_E, 5, 0.500),

        BW_create_sound_note(BW_D, 5, 0.250),
        BW_create_sound_note(BW_B, 4, 0.250),
        BW_create_sound_note(BW_G, 4, 0.250),
        BW_create_sound_note(BW_B, 4, 0.500),

        BW_create_sound_note(BW_C, 5, 0.250),
        BW_create_sound_note(BW_E, 5, 0.250),
        BW_create_sound_note(BW_G, 5, 0.250),
        BW_create_sound_note(BW_C, 6, 0.750),

        BW_create_sound_note(BW_B, 5, 0.250),
        BW_create_sound_note(BW_G, 5, 0.250),
        BW_create_sound_note(BW_E, 5, 0.250),
        BW_create_sound_note(BW_B, 4, 0.750),
    };

    int melody_len = sizeof(melody) / sizeof(melody[0]);

    for (int i = 0; i < melody_len; i++) {
        if (!melody[i]) {
            fprintf(stderr, "Failed to load note %d\n", i);
            BW_close();
            return 1;
        }

        BW_play_sound(melody[i]);
        sleep_ms((unsigned)(BW_get_sound_duration(melody[i]) * 1000.0 + 20.0));
    }

    BW_close();
    return 0;
}
