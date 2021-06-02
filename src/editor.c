#include <SDL2/SDL.h>
#include <stdbool.h>
#include "game.h"
#include "image.h"
#include "map.h"
#include "editor.h"

void editor()
{
	int editor_x = 0, editor_y = 0, editor_tile = 0;
	init_map();
	SDL_Event event;
	bool quit = false;
	int redraw = true;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

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
				case SDLK_UP:
					if(editor_y > 0)
					{
						editor_y--;
						redraw = true;
					}
					break;
				case SDLK_DOWN:
					if(editor_y < g_map_h-1)
					{
						editor_y++;
						redraw = true;
					}
					break;
				case SDLK_LEFT:
					if(editor_x > 0)
					{
						editor_x--;
						redraw = true;
					}
					break;
				case SDLK_RIGHT:
					if(editor_x < g_map_w-1)
					{
						editor_x++;
						redraw = true;
					}
					break;
				case SDLK_TAB:
				case SDLK_x:
					editor_tile++;
					if(editor_tile > 9)
						editor_tile = 0;
					redraw = true;
					break;
				case SDLK_RETURN:
					if(keyboard_state[SDL_SCANCODE_LALT])
					{
						toggle_fullscreen();
						redraw = 2;
						break;
					}
				case SDLK_SPACE:
				case SDLK_z:
					redraw = true;
					g_map[editor_y*g_map_w+editor_x] = editor_tile;
					break;

				case SDLK_w:
					save_map("data/level.lvl");
					draw_text(0, g_height-6, "saved");
					SDL_RenderPresent(g_renderer);
					break;
				case SDLK_ESCAPE:
				case SDLK_q:
					quit = true;
					break;
				}
				break;
			}
		}

		if(redraw)
		{
			SDL_RenderClear(g_renderer);
			for(int x = editor_x - g_width/16 - 1; x <= editor_x + g_width/16 + 1; x++)
				for(int y = editor_y - g_height/16 - 1; y <= editor_y + g_height/16 + 1; y++)
				{
					if(x < 0 || y < 0 || x >= g_map_w || y >= g_map_h)
						continue;
					int t = g_map[y*g_map_w+x];
					draw_texture_region(g_tileset, (t % 4)*8, (t / 4)*8, 8, 8, (-editor_x+x)*8+g_width/2-4, (-editor_y+y)*8+g_height/2-4, 8, 8);
				}
			draw_texture_region(g_tileset, 3*8, 7*8, 8, 8, g_width/2-4, g_height/2-4, 8, 8);
			draw_text(0, 0, "w:save q:quit\nx:switch\nz:place");
			draw_text(g_width-9*4-8, 0, "selected:");
			draw_texture_region(g_tileset, (editor_tile % 4)*8, (editor_tile / 4)*8, 8, 8, g_width-8, 0, 8, 8);
			draw_texture_region(g_tileset, 3*8, 7*8, 8, 8, g_width-8, 0, 8, 8);
			char coords[25];
			sprintf(coords, "x:%d y:%d", editor_x, editor_y);
			draw_text(g_width - strlen(coords)*4, g_height - 6, coords);
			SDL_RenderPresent(g_renderer);

			redraw--;
		}
	}

	free(g_map);
}
