#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "audio.h"
#include "ensure.h"

Uint8 *g_audio_pos, *g_audio_buf;
Uint32 g_audio_len;
SDL_AudioSpec g_wav_spec;

struct audio_wav g_sfx_chop, g_sfx_throw, g_sfx_open, g_sfx_select, g_sfx_item, g_sfx_zap, g_sfx_shoot;

void audio_callback(void *userdata, Uint8 *stream, int len)
{
	if(g_audio_len == 0)
	{
		g_audio_pos = NULL;
		SDL_PauseAudio(1);
		return;
	}
	len = (len > g_audio_len ? g_audio_len : len);
	SDL_memcpy(stream, g_audio_pos, (Uint32)len);
	g_audio_pos += (Uint32)len;
	g_audio_len -= (Uint32)len;
}

bool load_wav(struct audio_wav *dest, const char *filename)
{
	if(SDL_LoadWAV(filename, &g_wav_spec, &dest->buf, &dest->len) == NULL)
	{
		fprintf(stderr, "failed to load wav! sdl error: %s\n", SDL_GetError());
		return false;
	}
	return true;
}

void free_wav(struct audio_wav *wav)
{
	SDL_FreeWAV(wav->buf);
}

void init_audio()
{
	ensure(load_wav(&g_sfx_chop, "data/sfx/chop.wav"), "chop.wav");
	ensure(load_wav(&g_sfx_throw, "data/sfx/throw.wav"), "throw.wav");
	ensure(load_wav(&g_sfx_open, "data/sfx/open.wav"), "open.wav");
	ensure(load_wav(&g_sfx_item, "data/sfx/item.wav"), "item.wav");
	ensure(load_wav(&g_sfx_select, "data/sfx/select.wav"), "select.wav");
	ensure(load_wav(&g_sfx_zap, "data/sfx/zap.wav"), "zap.wav");
	ensure(load_wav(&g_sfx_shoot, "data/sfx/shoot.wav"), "shoot.wav");

	g_wav_spec.callback = audio_callback;
	g_wav_spec.userdata = NULL;

	SDL_AudioSpec desired;
	SDL_zero(desired);
	desired.freq = 22050;
	desired.format = AUDIO_U8;
	desired.channels = 2;
	desired.samples = 1024;
	
	ensure((SDL_OpenAudio(&g_wav_spec, &desired) >= 0), "audio device");
	g_audio_pos = NULL;
}

void end_audio()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	free_wav(&g_sfx_shoot);
	free_wav(&g_sfx_zap);
	free_wav(&g_sfx_select);
	free_wav(&g_sfx_item);
	free_wav(&g_sfx_open);
	free_wav(&g_sfx_throw);
	free_wav(&g_sfx_chop);
}

void play_audio(struct audio_wav wav)
{
	if(g_audio_pos != NULL || g_audio_len > 0)
	{
		g_audio_pos = NULL;
		g_audio_len = 0;
	}
	g_audio_pos = wav.buf;
	g_audio_len = wav.len;
	SDL_PauseAudio(0);
}
