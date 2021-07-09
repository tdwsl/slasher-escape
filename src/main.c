#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "audio.h"
#include "image.h"
#include "editor.h"
#include "map.h"
#include "game.h"

void main_menu()
{
	SDL_Event event;
	bool quit = false, redraw = true, confirm = false, help = false;
	int last_update = SDL_GetTicks(), cursor_step = 0, cursor_y = 0, cursor_yo = 0;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

	while(!quit && !g_quit)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				g_quit = true;
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
				case SDLK_q:
					if(help) help = false;
					break;
				case SDLK_ESCAPE:
					if(help)
						help = false;
					else
						quit = true;
					break;
				case SDLK_UP:
					if(cursor_yo == 0 && cursor_y > 0 && !help)
					{
						cursor_yo = -1;
						play_audio(g_sfx_select);
					}
					break;
				case SDLK_DOWN:
					if(cursor_yo == 0 && cursor_y < 3 && !help)
					{
						cursor_yo = 1;
						play_audio(g_sfx_select);
					}
					break;
				case SDLK_RETURN:
					if(keyboard_state[SDL_SCANCODE_LALT])
					{
						toggle_fullscreen();
						redraw = true;
						break;
					}
				case SDLK_SPACE:
				case SDLK_z:
				case SDLK_0:
					if(cursor_yo == 0)
						confirm = true;
					break;
				}
			}
		}

		if(redraw)
		{
			SDL_SetRenderDrawColor(g_renderer, 0x10, 0x10, 0x10, 0xff);
			SDL_RenderClear(g_renderer);
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
			draw_texture_region(g_vigenette, 0, 0, 160, 120, 0, 0, g_width, g_height);
			if(help)
			{
				draw_text(10, 10, "controls:\nz/0: interact / use\nx/equals: switch item / hold to throw\narrows: movement");
				draw_text(10, g_height-10-9*7, "how to play:");
				draw_text(10, g_height-10-9*6, "search cabinets");
				draw_texture_region(g_tileset, 3*8, 1*8, 8, 8, 10, g_height-10-9*5, 8, 8);
				draw_text(10, g_height-10-9*4, "find car parts");
				for(int i = 0; i < 3; i++)
					draw_texture_region(g_tileset, ((i*2)%4)*8, 4*8+(i*2/4)*8, 8, 8, 10+i*9, g_height-10-9*3, 8, 8);
				draw_text(10, g_height-10-9*2, "escape");
				draw_texture_region(g_tileset, 0, 2*8, 16, 8, 10, g_height-10-9*1, 16, 8);
				draw_text(g_width-10-18*4, g_height-10-9*7, "weapons:");
				char name[3][10] = {
					"shotgun",
					"axe",
					"taser",
				};
				char desc[3][30] = {
					"shoots killer",
					"chops trees/shields",
					"stuns killer",
				};
				for(int i = 0; i < 3; i++)
				{
					draw_texture_region(g_tileset, ((i*2+1)%4)*8, 4*8+((i*2+1)/4)*8, 8, 8, g_width-10-18*4, g_height-10-9*(6-i*2), 8, 8);
					draw_text(g_width-10-18*4+9, g_height-10-9*(6-i*2), name[i]);
					draw_text(g_width-10-18*4, g_height-10-9*(6-i*2-1), desc[i]);
				}
			}
			else
			{
				draw_text(g_width/2 - 7*4, 6, "slasher escape");
				draw_text(g_width/2 - 4*4, g_height/2-2*6, "new game\nhow to play\neditor\nquit");
				draw_texture_region(g_tileset, (bool)(cursor_step / 10)*8, 8*8, 8, 8, g_width/2-8*4, g_height/2-2*6 + cursor_y*6+cursor_yo-1, 8, 8);
				if(!g_fullscreen)
					draw_text(g_width-20*4, g_height-6, "alt+enter:fullscreen");
			}
			SDL_RenderPresent(g_renderer);
			redraw = false;
		}

		int current_time = SDL_GetTicks();
		if(current_time - last_update > 20)
		{
			int ps = cursor_step;
			cursor_step++;
			if(cursor_step > 20)
				cursor_step = 0;
			if(ps/10 != cursor_step/10)
				redraw = true;

			if(cursor_yo > 0)
			{
				cursor_yo++;
				redraw = true;
			}
			if(cursor_yo < 0)
			{
				cursor_yo--;
				redraw = true;
			}
			if(cursor_yo >= 6)
			{
				cursor_y++;
				cursor_yo = 0;
			}
			if(cursor_yo <= -6)
			{
				cursor_y--;
				cursor_yo = 0;
			}

			last_update = current_time;
		}

		if(confirm)
		{
			play_audio(g_sfx_item);
			switch(cursor_y)
			{
			case 0:
				play_game();
				break;
			case 1:
				help = !help;
				break;
			case 2:
				editor();
				break;
			case 3:
				g_quit = true;
				break;
			}

			confirm = false;
		}
	}
}

void play_intro()
{
	SDL_RenderClear(g_renderer);
	draw_text(g_width/2-7*4, g_height/2-2.5*6, "slasher escape");
	SDL_RenderPresent(g_renderer);
	SDL_Delay(750);
	draw_text(g_width/2-12*4, g_height/2-0.5*6, "a game made by tyler dwsl\n  for the 4mb gamejam");
	SDL_RenderPresent(g_renderer);
	SDL_Delay(1500);
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_QUIT:
			g_quit = true;
			break;
		}
	}
}

int main(int argc, char *args[])
{
	time_t t;
	srand(time(&t));

#ifdef __MINGW32__
	putenv("SDL_AUDIODRIVER=directsound");
#endif

	init_sdl();
	init_audio();
	play_intro();
	main_menu();
	end_audio();
	end_sdl();
	return 0;
}
