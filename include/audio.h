#ifndef SLASHER_AUDIO
#define SLASHER_AUDIO

#include <SDL2/SDL.h>

extern Uint8 *g_audio_pos, *g_audio_buf;
extern Uint32 g_audio_len;
extern SDL_AudioSpec g_wav_spec;

struct audio_wav
{
	Uint32 len;
	Uint8 *buf;
};

extern struct audio_wav g_sfx_chop, g_sfx_throw, g_sfx_open, g_sfx_select, g_sfx_item, g_sfx_zap, g_sfx_shoot;

void init_audio();
void end_audio();
void play_audio(struct audio_wav wav);

#endif
