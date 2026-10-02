#include "bitwave.h"
#include "deps/miniaudio.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BW_NOTE_AMPLITUDE      0.2f
#define BW_NOTE_FADE_SECONDS   0.005
#define BW_DEFAULT_SAMPLE_RATE 44100

typedef enum {
    BW_sound_source_file,
    BW_sound_source_memory,
    BW_sound_source_waveform
} BW_SoundSource;

struct BW_Sound {
    ma_sound sound;
    union {
        ma_decoder  decoder;
        ma_waveform waveform;
    } source;
    void*             data; 
    size_t            data_size;
    double            duration;
    double            frequency;
    BW_SoundSource kind;
};

static ma_engine g_engine;
static int       g_initialized = 0;

/* Engine lifecycle */

int BW_init(void) {
    if (g_initialized) return 1;
    if (ma_engine_init(NULL, &g_engine) != MA_SUCCESS) return 0;
    g_initialized = 1;
    return 1;
}

void BW_close(void) {
    if (!g_initialized) return;
    ma_engine_uninit(&g_engine);
    g_initialized = 0;
}

static ma_uint32 BW_sample_rate(void) {
    ma_uint32 rate = ma_engine_get_sample_rate(&g_engine);
    return rate == 0 ? BW_DEFAULT_SAMPLE_RATE : rate;
}

/* Zeroed allocation so no field is ever left uninitialized. */
static BW_Sound* BW_alloc(BW_SoundSource kind) {
    BW_Sound* s = (BW_Sound*)calloc(1, sizeof(*s));
    if (s) s->kind = kind;
    return s;
}

static BW_Sound* BW_create_from_memory(const void* data, size_t size) {
    BW_Sound* s = BW_alloc(BW_sound_source_memory);
    if (!s) return NULL;

    s->data = malloc(size);
    if (!s->data) { free(s); return NULL; }
    memcpy(s->data, data, size);
    s->data_size = size;

    ma_decoder_config cfg = ma_decoder_config_init(ma_format_unknown, 0, 0);
    if (ma_decoder_init_memory(s->data, size, &cfg, &s->source.decoder) != MA_SUCCESS) {
        free(s->data);
        free(s);
        return NULL;
    }

    if (ma_sound_init_from_data_source(&g_engine, &s->source.decoder,
                                       0, NULL, &s->sound) != MA_SUCCESS) {
        ma_decoder_uninit(&s->source.decoder);
        free(s->data);
        free(s);
        return NULL;
    }
    return s;
}

static BW_Sound* BW_create_waveform(double frequency, double duration) {
    BW_Sound* s = BW_alloc(BW_sound_source_waveform);
    if (!s) return NULL;

    s->duration  = duration < 0.0 ? 0.0 : duration;
    s->frequency = frequency;

    ma_waveform_config cfg = ma_waveform_config_init(
        ma_format_f32, 1, BW_sample_rate(),
        ma_waveform_type_sine, BW_NOTE_AMPLITUDE, frequency);

    if (ma_waveform_init(&cfg, &s->source.waveform) != MA_SUCCESS) {
        free(s);
        return NULL;
    }

    if (ma_sound_init_from_data_source(&g_engine, &s->source.waveform,
                                       MA_SOUND_FLAG_STREAM, NULL, &s->sound) != MA_SUCCESS) {
        ma_waveform_uninit(&s->source.waveform);
        free(s);
        return NULL;
    }
    return s;
}

/* Loading */
BW_Sound* BW_load_sound(const char* path) {
    if (!g_initialized || !path) return NULL;

    BW_Sound* s = BW_alloc(BW_sound_source_file);
    if (!s) return NULL;

    if (ma_sound_init_from_file(&g_engine, path, 0, NULL, NULL, &s->sound) != MA_SUCCESS) {
        free(s);
        return NULL;
    }
    return s;
}

BW_Sound* BW_create_sound_from_memory(const void* data, size_t data_size) {
    if (!g_initialized || !data || data_size == 0) return NULL;
    return BW_create_from_memory(data, data_size);
}

BW_Sound* BW_create_sound_note(int note, int octave, double duration) {
    if (!g_initialized) return NULL;

    double semitones = (double)(note - BW_A + 12 * (octave - 4));
    double frequency = 440.0 * exp2(semitones / 12.0);

    return BW_create_waveform(frequency, duration);
}

/* Duration-less tone: keeps sounding until BW_stop_sound(). */
BW_Sound* BW_create_sound_wave(double frequency) {
    if (!g_initialized || frequency <= 0.0) return NULL;
    return BW_create_waveform(frequency, 0.0);
}

/* Copying */
BW_Sound* BW_copy_sound(const BW_Sound* src) {
    if (!g_initialized || !src) return NULL;

    switch (src->kind) {
    case BW_sound_source_file: {
        BW_Sound* s = BW_alloc(BW_sound_source_file);
        if (!s) return NULL;
        if (ma_sound_init_copy(&g_engine, &src->sound, 0, NULL, &s->sound) != MA_SUCCESS) {
            free(s);
            return NULL;
        }
        return s;
    }
    case BW_sound_source_memory:
        return BW_create_from_memory(src->data, src->data_size);
    case BW_sound_source_waveform:
        return BW_create_waveform(src->frequency, src->duration);
    }
    return NULL;
}

/* Queries */

double BW_get_sound_duration(const BW_Sound* sound) {
    if (!sound) return 0.0;
    if (sound->kind == BW_sound_source_waveform) return sound->duration;

    float seconds = 0.0f;
    if (ma_sound_get_length_in_seconds((ma_sound*)&sound->sound, &seconds) != MA_SUCCESS) {
        return 0.0;
    }
    return (double)seconds;
}

/* Playback */
void BW_play_sound(BW_Sound* sound) {
    if (!sound) return;

    if (sound->kind == BW_sound_source_waveform) {
        if (sound->duration > 0.0) {
            ma_uint32 rate   = BW_sample_rate();
            ma_uint64 length = (ma_uint64)(sound->duration * rate);
            ma_uint64 fade   = (ma_uint64)(BW_NOTE_FADE_SECONDS * rate);
            if (fade > length) fade = length;

            ma_uint64 stop = ma_engine_get_time_in_pcm_frames(&g_engine) + length;
            ma_sound_set_stop_time_with_fade_in_pcm_frames(&sound->sound, stop, fade);
        }
    } else if (ma_sound_at_end(&sound->sound)) {
        ma_sound_seek_to_pcm_frame(&sound->sound, 0);
    }

    ma_sound_start(&sound->sound);
}

void BW_stop_sound(BW_Sound* sound) {
    if (!sound) return;
    ma_sound_stop(&sound->sound);
}

void BW_free_sound(BW_Sound* sound) {
    if (!sound) return;

    ma_sound_uninit(&sound->sound);

    switch (sound->kind) {
    case BW_sound_source_memory:
        ma_decoder_uninit(&sound->source.decoder);
        free(sound->data);
        break;
    case BW_sound_source_waveform:
        ma_waveform_uninit(&sound->source.waveform);
        break;
    case BW_sound_source_file:
        break;
    }

    free(sound);
}

int BW_play(const char* path) {
    if (!g_initialized || !path) return 0;
    return ma_engine_play_sound(&g_engine, path, NULL) == MA_SUCCESS;
}


void BW_set_volume(BW_Sound* sound, float volume) {
    ma_sound_set_volume(&sound->sound, volume);
}

void BW_set_pitch(BW_Sound* sound, float pitch) {
    ma_sound_set_pitch(&sound->sound, pitch);
}
