#ifndef BW_H
#define BW_H

#include <stddef.h>

#define BW_C   0
#define BW_Cs  1
#define BW_Db  1
#define BW_D   2
#define BW_Ds  3
#define BW_Eb  3
#define BW_E   4
#define BW_F   5
#define BW_Fs  6
#define BW_Gb  6
#define BW_G   7
#define BW_Gs  8
#define BW_Ab  8
#define BW_A   9
#define BW_As  10
#define BW_Bb  10
#define BW_B   11

typedef struct BW_Sound BW_Sound;

int BW_init(void);
void BW_close(void);

BW_Sound* BW_load_sound(const char* path);
BW_Sound* BW_create_sound_from_memory(const void* data, size_t data_size);
BW_Sound* BW_create_sound_note(int note, int octave, double duration);
BW_Sound* BW_create_sound_wave(double frequency);
BW_Sound* BW_copy_sound(const BW_Sound* src);

double BW_get_sound_duration(const BW_Sound* sound);

void BW_play_sound(BW_Sound* sound);
void BW_stop_sound(BW_Sound* sound);
void BW_free_sound(BW_Sound* sound);

void BW_set_pitch(BW_Sound* sound, float pitch);
void BW_set_volume(BW_Sound* sound, float volume);


int BW_play(const char* path);

#endif
