#include <SDL2/SDL.h>
#include <stdbool.h>
#include "image.h"
#include "victory.h"
#include "game.h"
#include "audio.h"

void victory()
{
	play_audio(g_sfx_item);
	SDL_Event event;
	int last_update = SDL_GetTicks(), frame = 0;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	bool quit = false, redraw = true;

	while(!quit)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				quit = true;
				g_quit = true;
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
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
					if(frame < 20)
						break;
				case SDLK_q:
				case SDLK_ESCAPE:
					quit = true;
					break;
				}
				break;
			}
		}

		int current_time = SDL_GetTicks();
		if(current_time - last_update > 200)
		{
			frame++;
			if(frame > 103) frame = 100;
			last_update = current_time;
		}

		if(redraw)
		{
			SDL_RenderClear(g_renderer);
			draw_texture_region(g_victory, 0, 0, 160, 60, 0, g_height-60+2+frame, g_width, 60);
			draw_texture_region(g_victory, 0, 60, 160, 60, g_width/2-80, g_height-60+2+(frame % 3), 160, 60);
			if(frame > 20) draw_text(g_width/2-3.5*4, g_height/2-3, "you win");
			SDL_RenderPresent(g_renderer);
		}
	}
}
