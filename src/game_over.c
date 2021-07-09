#include <SDL2/SDL.h>
#include <stdbool.h>
#include "game_over.h"
#include "game.h"
#include "image.h"
#include "audio.h"

void game_over()
{
	SDL_Event event;
	int last_update = SDL_GetTicks();
	bool quit = false, redraw = true;
	int frame = 0, stage = 0;
	int max_frames[5] = {12, 20, 25, 20, 30};
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PauseAudio(1);
	g_audio_len = 0;
	g_audio_pos = NULL;

	while(!quit)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				g_quit = true;
				quit = true;
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
				case SDLK_q:
					quit = true;
					break;
				case SDLK_SPACE:
				case SDLK_RETURN:
					if(keyboard_state[SDL_SCANCODE_LALT])
					{
						toggle_fullscreen();
						break;
					}
				case SDLK_z:
				case SDLK_x:
				case SDLK_0:
				case SDLK_EQUALS:
					if(stage > 0)
						quit = true;
					break;
				}
				break;
			}
		}

		if(redraw)
		{
			SDL_RenderClear(g_renderer);
			float mul = (float)frame/max_frames[stage];
			switch(stage)
			{
			case 0:
				draw_texture_region(g_gameover, 0, 0, 40, 60, g_width-20-mul*g_width/2, g_height/2-50, 40, 60);
				draw_texture_region(g_gameover, 0, 60, 40, 60, mul*g_width/2-20, g_height-60+2, 40, 60);
				break;
			case 1:
				draw_texture_region(g_gameover, 40, 0, 40, 60, g_width/2-20, g_height/2-50-mul*10, 40, 60);
				draw_texture_region(g_gameover, 40, 60, 40, 60, g_width/2-20, g_height-60+2+mul*15, 40, 60);
				break;
			case 2:
				draw_texture_region(g_gameover, 80, 0, 40, 60, g_width/2-20, g_height/2-30+mul*5, 40, 60);
				draw_texture_region(g_gameover, 80, 60, 40, 60, g_width/2-20, g_height-60+2+5-mul*5, 40, 60);
				break;
			case 3:
				draw_texture_region(g_gameover, 80, 0, 40, 60, g_width/2-20, g_height-60-30-mul*20, 40, 60);
				draw_texture_region(g_gameover, 120, 60, 40, 60, g_width/2-20, g_height-60+2, 40, 60);
				break;
			case 4:
				draw_texture_region(g_gameover, 120, 0, 40, 60, g_width/2-20, g_height/2-60, 40, 60);
				draw_texture_region(g_gameover, 120, 60, 40, 60, g_width/2-20, g_height-60+2, 40, 60);
				draw_text(g_width/2-4.5*4, g_height/2-3, "game over");
				break;
			}
			SDL_RenderPresent(g_renderer);
		}

		int current_time = SDL_GetTicks();
		if(current_time - last_update > 30)
		{
			if(stage < 4)
			{
				frame++;
				if(frame > max_frames[stage])
				{
					stage++;
					frame = 0;
					if(stage == 2)
					{
						SDL_SetRenderDrawColor(g_renderer, 0xff, 0x00, 0x00, 0xff);
						play_audio(g_sfx_chop);
					}
				}
			}

			redraw = true;

			last_update = current_time;
		}
	}

	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
}
