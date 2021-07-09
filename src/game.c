#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "map.h"
#include "game.h"
#include "item.h"
#include "actor.h"
#include "game_over.h"
#include "image.h"
#include "victory.h"
#include "audio.h"

#define TOTAL_ITEMS 6

int g_car_parts;
bool g_quit = false;

void play_game()
{
	SDL_Event event;
	struct actor player, killer;
	init_map();
	spawn_actor(&player, 0);
	init_actor(&killer, 0, 0, 1);
	teleport_actor_near(&killer, player.x+4, player.y+4);
	bool quit = false, redraw = true, killer_chasing = false, player_full = false, killer_ko = false, killer_smashing = false, killer_tazed = false;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	int last_update = SDL_GetTicks(), camera_x = player.x + 4, camera_y = player.y + 4, car_x, car_y, killer_cooldown = 0, ammo = 3;
	int killer_chase_counter = 200, cabinet_count = 0, items_left = TOTAL_ITEMS, player_selected = 0, throw_charge = 0, killer_stun = 0, taser_cooldown = 0;
	int player_items[3] = {-1, -1, -1};
	bool items_available[TOTAL_ITEMS];
	char item_names[TOTAL_ITEMS][25] = {
		"car key",
		"shotgun",
		"gas can",
		"axe",
		"car battery",
		"taser",
	};
	for(int i = 0; i < TOTAL_ITEMS; i++)
		items_available[i] = true;
	g_car_parts = 0;
	for(int i = 0; i < MAX_STUFF; i++)
		g_items[i].type = -1;
	for(int x = 0; x < g_map_w; x++)
		for(int y = 0; y < g_map_h; y++)
		{
			int t = g_map[y*g_map_w+x];
			if(t == 7) cabinet_count++;
			if(t == 8 || t == 9)
			{
				car_x = x*8;
				car_y = y*8;
			}
		}

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
				case SDLK_RETURN:
					if(keyboard_state[SDL_SCANCODE_LALT])
						toggle_fullscreen();
					redraw = true;
					break;
				case SDLK_UP:
					player.yv = -1;
					break;
				case SDLK_DOWN:
					player.yv = 1;
					break;
				case SDLK_LEFT:
					player.xv = -1;
					break;
				case SDLK_RIGHT:
					player.xv = 1;
					break;
				}
				break;
			case SDL_KEYUP:
				switch(event.key.keysym.sym)
				{
				case SDLK_z:
				case SDLK_0:
					if(pow(car_x-player.x, 2) + pow(car_y-player.y, 2) < 20*20 && g_car_parts >= 3)
					{
						victory();
						play_audio(g_sfx_item);
						redraw = false;
						quit = true;
						break;
					}
					if(player.near_item != -1 && !player_full)
					{
						int i;
						for(i = 0; i < 3; i++)
							if(player_items[i] == -1) break;
						if(player_items[player_selected] == -1)
							i = player_selected;
						player_items[i] = g_items[player.near_item].type;
						delete_item(&g_items[player.near_item]);
						for(i = 0; i < 3; i++)
							if(player_items[i] == -1) break;
						if(i >= 3) player_full = true;
						play_audio(g_sfx_item);
						redraw = true;
						break;
					}
					if(player.near_open_door||player.near_closed_door||player.near_closed_cabinet)
					{
						if(player.near_closed_cabinet)
						{
							int ch;
							ch = 40;
							if(cabinet_count <= items_left) ch = 99;
							if(ch >= rand() % 100 && items_left > 0)
							{
								int t;
								while(1)
								{
									t = rand() % TOTAL_ITEMS;
									if(items_available[t])
										break;
								}
								items_available[t] = false;
								add_item(player.x+4, player.y+4, t);
								items_left--;
							}
							cabinet_count--;
						}
						actor_interact_tile(player);
						redraw = true;
						break;
					}
					if(player_items[player_selected] == 3 && player.near_tree)
					{
						actor_chop(player);
						play_audio(g_sfx_chop);
						redraw = true;
						break;
					}
					if(player_items[player_selected] == 1 || player_items[player_selected] == 5)
					{
						if(player_items[player_selected] == 1)
						{
							if(ammo > 0)
								ammo--;
							else
								break;
							play_audio(g_sfx_shoot);
						}
						if(player_items[player_selected] == 5)
						{
							if(taser_cooldown > 0)
								break;
							taser_cooldown = 450;
							play_audio(g_sfx_zap);
						}
						bool destroys = (player_items[player_selected] == 1);
						bool result = actor_shoot(player, destroys, killer.x+4, killer.y+4);
						if(result)
						{
							if(player_items[player_selected] == 1)
							{
								killer_ko = true;
								killer_stun = 750;
							}
							if(player_items[player_selected] == 5)
							{
								killer_stun = 75;
								killer_tazed = true;
								if(tile_at(killer.x+4, killer.y+4) == 4)
								{
									killer_stun *= 2;
									killer_ko = true;
								}
							}
						}
						redraw = true;
					}
					break;
				case SDLK_x:
				case SDLK_EQUALS:
					if(throw_charge > 5)
					{
						throw_charge = 0;
						break;
					}
					throw_charge = 0;
					player_selected++;
					if(player_selected >= 3)
						player_selected = 0;
					play_audio(g_sfx_select);
					redraw = true;
					break;
				case SDLK_UP:
					if(player.yv == -1) player.yv = 0;
					break;
				case SDLK_DOWN:
					if(player.yv == 1) player.yv = 0;
					break;
				case SDLK_LEFT:
					if(player.xv == -1) player.xv = 0;
					break;
				case SDLK_RIGHT:
					if(player.xv == 1) player.xv = 0;
					break;
				}
				break;
			}
		}

		int current_time = SDL_GetTicks();
		if(current_time - last_update > 30)
		{
			int o_car_parts = g_car_parts;
			for(int i = 0; g_items[i].type != -1; i++)
				if(update_item(&g_items[i]))
				{
					redraw = true;
					if(o_car_parts != g_car_parts)
					{
						delete_item(&g_items[i]);
						i--;
						continue;
					}
				}

			if((keyboard_state[SDL_SCANCODE_X] || keyboard_state[SDL_SCANCODE_EQUALS]) && player_items[player_selected] != -1)
			{
				throw_charge++;
				if(throw_charge > 5)
				{
					actor_throw_item(&player, player_items[player_selected]);
					player_items[player_selected] = -1;
					play_audio(g_sfx_throw);
					player_full = false;
					redraw = true;
				}
			}

			if(taser_cooldown > 0)
			{
				taser_cooldown--;
				redraw = true;
			}

			if(killer_stun > 0)
			{
				killer_stun--;
				if(killer_tazed)
					redraw = true;
				if(killer_stun <= 0)
				{
					killer_ko = false;
					killer_tazed = false;
					redraw = true;
				}
			}
			else
			{
				if(killer_chase_counter > 0)
					killer_chase_counter--;
				else
				{
					killer_chasing = !killer_chasing;
					if(killer_chasing)
					{
						killer_chase_counter = 150 + rand() % 75;
						killer_smashing = (TOTAL_ITEMS - items_left > 0);
					}
					else
						killer_chase_counter = 100 - rand() % 50;
				}
				if(killer_cooldown > 0)
					killer_cooldown--;
				killer.xv = 0;
				killer.yv = 0;
				if(killer.x < player.x) killer.xv = 1;
				if(killer.x > player.x) killer.xv = -1;
				if(killer.y < player.y) killer.yv = 1;
				if(killer.y > player.y) killer.yv = -1;
				if(!killer_chasing)
				{
					killer.yv *= -1;
					killer.xv *= -1;
				}
				if((killer.near_window||killer.near_closed_door) && killer_chasing && killer_smashing && killer_cooldown <= 0)
				{
					actor_smash(killer);
					play_audio(g_sfx_chop);
					killer_cooldown = 75;
					redraw = true;
				}
				if(tile_at(killer.x+4, killer.y+4) == 4)
					killer.speed = 1.75;
				else
					killer.speed = 1.25;
				if(!killer_chasing)
					killer.speed *= 0.5;
				update_actor(&killer);
				if(pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) < 50*50)
					redraw = true;
				else if(pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) > 160*160 && killer_chasing && killer_stun <= 0)
					teleport_actor_near(&killer, player.x, player.y);

				for(int i = 0; g_items[i].type != -1; i++)
				{
					struct item *it = &g_items[i];
					if(pow(killer.x+4-it->x, 2) + pow(killer.y+4-it->y, 2) < 8*8 && (it->xv != 0 || it->yv != 0))
					{
						killer_stun = 25;
						if(it->type == 3)
							killer_stun = 55;
						killer_ko = true;
						move_actor(&killer, it->xv, it->yv);
						it->xv = 0;
						it->yv = 0;
						break;
					}
				}
			}

			if(tile_at(player.x+4, player.y+4) == 4)
				player.speed = 1;
			else
				player.speed = 1.5;
			update_actor(&player);
			if(player.xv != 0 || player.yv != 0)
				redraw = true;

			int xm, ym, ocx, ocy;
			ocx = camera_x;
			ocy = camera_y;
			xm = abs(camera_x - player.x - 4) / 20;
			ym = abs(camera_y - player.y - 4) / 15;

			if(camera_x < player.x+4-1) camera_x += xm;
			if(camera_y < player.y+4) camera_y += ym;
			if(camera_x > player.x+4) camera_x -= xm;
			if(camera_y > player.y+4) camera_y -= ym;

			if(camera_x - g_width/2 < 0) camera_x = g_width/2;
			if(camera_y - g_height/2 < 0) camera_y = g_height/2;
			if(camera_x + g_width/2 >= g_map_w*8) camera_x = g_map_w*8 - g_width/2 - 1;
			if(camera_y + g_height/2 >= g_map_h*8) camera_y = g_map_h*8 - g_height/2 - 1;

			if(camera_x != ocx || camera_y != ocy) redraw = true;

			if((pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) < 7*7) && killer_stun <= 0)
			{
				if(player_items[player_selected] == 3)
				{
					player_items[player_selected] = -1;
					play_audio(g_sfx_chop);
					killer_stun = 500;
					killer_ko = true;
					move_actor(&killer, killer.xv*-2, killer.yv*-2);
					player_full = false;
				}
				else
				{
					game_over();
					quit = true;
					redraw = false;
				}
			}

			if(redraw)
			{
				SDL_RenderClear(g_renderer);
				int xo = camera_x-g_width/2;
				int yo = camera_y-g_height/2;
				for(int x = (player.x+4-40)/8-1; x < (player.x+4+40)/8+1; x++)
					for(int y = (player.y+4-30)/8-1; y < (player.y+4+30)/8+1; y++)
					{
						if(y < 0 || x < 0 || y >= g_map_h || x >= g_map_w)
							continue;
						int t = g_map[y*g_map_w+x];
						draw_texture_region(g_tileset, (t % 4)*8, (t / 4)*8, 8, 8, x*8-xo, y*8-yo, 8, 8);
					}
				for(int i = 0; g_items[i].type != -1; i++)
					if(pow(player.x+4-g_items[i].x, 2) + pow(player.y+4-g_items[i].y, 2) < 50*50)
						draw_item(g_items[i], xo, yo);
				if(pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) < 50*50)
				{
					if(killer_ko)
						draw_texture_region(g_tileset, 2*8, 7*8, 8, 8, killer.x-xo, killer.y-yo, 8, 8);
					else
						draw_actor(killer, xo, yo);
					if(killer_tazed)
						draw_texture_region(g_tileset, 8*(killer_stun % 2), 6*8, 8, 8, killer.x-xo, killer.y-yo, 8, 8);
				}
				draw_actor(player, xo, yo);
				bool arrow = false;
				if(player_items[player_selected] != -1)
				{
					int t = player_items[player_selected];
					draw_texture_region(g_tileset, (t % 4)*8, 4*8+(t / 4)*8, 8, 8, player.x-xo, player.y-yo, 8, 8);
					if(t % 2 == 0 && pow(player.x-car_x, 2) + pow(player.y-car_y, 2) > 30*30)
						arrow = true;
				}
				if(pow(player.x-car_x, 2) + pow(player.y-car_y, 2) < 50*50)
				{
					char text[4];
					sprintf(text, "%d/3", g_car_parts);
					draw_text(car_x-4*1.5-xo, car_y-1-6-yo, text);
				}
				if(pow(car_x-player.x, 2) + pow(car_y-player.y, 2) < 20*20 && g_car_parts >= 3)
					draw_text(player.x-xo-4*4, player.y-yo-6-1, "z:escape");
				else if(player.near_item != -1 && !player_full)
					draw_text(player.x-xo-4.5*4, player.y-yo-6-1, "z:take item");
				else if(player.near_open_door)
					draw_text(player.x-xo-5*4, player.y-yo-6-1, "z:close door");
				else if(player.near_closed_door)
					draw_text(player.x-xo-5.5*4, player.y-yo-6-1, "z:open door");
				else if(player.near_closed_cabinet)
					draw_text(player.x-xo-4*4, player.y-yo-6-1, "z:search");
				else if((player_items[player_selected] == 1 && ammo > 0) || (player_items[player_selected] == 5 && taser_cooldown <= 0))
					draw_text(player.x-xo-3*4, player.y-yo-6-1, "z:shoot");
				else if(player.near_tree && player_items[player_selected] == 3)
					draw_text(player.x-xo-5*4, player.y-yo-6-1, "z:chop tree");
				draw_texture_region(g_vigenette, 0, 0, 160, 120, player.x+4-80-xo, player.y+4-60-yo, 160, 120);
				if(arrow)
				{
					float a = atan2(car_y-player.y-4, car_x-player.x-4);
					int r;
					for(r = 0; a > (PI/4)*r; r++);
					r = (r+1) % 8;
					draw_texture_region(g_arrow, r*8, 0, 8, 8, player.x+4+cosf(a)*25-xo-4, player.y+4+sinf(a)*25-yo-4, 8, 8);
				}
				for(int i = 0; i < 3; i++)
				{
					SDL_SetRenderDrawColor(g_renderer, 0x38, 0x50, 0x20, 0xff);
					draw_box(1+i*9, 1, 8, 8);
					SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
					int t = player_items[i];
					if(t != -1)
					{
						draw_texture_region(g_tileset, (t % 4)*8, 4*8+(t / 4)*8, 8, 8, 1+i*9, 1, 8, 8);
						if(t == 1)
						{
							for(int j = 0; j < ammo; j++)
							{
								SDL_SetRenderDrawColor(g_renderer, 0x88, 0x00, 0x00, 0xff);
								draw_box(1+i*9+j*2, 1+6, 1, 3);
								SDL_SetRenderDrawColor(g_renderer, 0x80, 0x6f, 0x00, 0xff);
								draw_pixel(1+i*9+j*2, 1+6);
							}
							SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
						}
						if(t == 5)
						{
							if(taser_cooldown > 0)
								SDL_SetRenderDrawColor(g_renderer, 0xff, 0x00, 0x00, 0xff);
							else
								SDL_SetRenderDrawColor(g_renderer, 0x00, 0xff, 0x00, 0xff);
							draw_box(1+i*9+1, 1+6, ((float)(450-taser_cooldown)/450)*5+1, 1);
							SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
						}
					}
				}
				if(player_items[player_selected] != -1)
					draw_text(1+3*9, 1, item_names[player_items[player_selected]]);
				draw_texture_region(g_tileset, 2*8, 5*8, 8, 8, 1+player_selected*9, 1+8, 8, 8);
				SDL_RenderPresent(g_renderer);

				redraw = false;
			}

			last_update = current_time;
		}

	}

	free(g_map);
}
