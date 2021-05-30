#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WINDOW_W 640
#define WINDOW_H 480
#define MAX_STUFF 25
#define PI 3.14159

SDL_Renderer *g_renderer = NULL;
SDL_Window *g_window = NULL;
SDL_Texture *g_tileset = NULL;
SDL_Texture *g_font = NULL;
SDL_Texture *g_vigenette = NULL;
SDL_Texture *g_gameover = NULL;
int g_scale = 4;
int g_width = 160, g_height = 120;
bool g_quit = false, g_fullscreen = false;

struct actor
{
	int xv, yv, skin, d, near_item;
	float x, y, speed;
	bool near_open_door, near_closed_door, near_closed_cabinet, near_window;
};

struct item
{
	int type;
	float x, y, xv, yv;
};

int *g_map;
struct item g_items[MAX_STUFF];
int g_map_w, g_map_h;

void ensure(bool cond, const char *desc)
{
	if(cond) return;
	printf("failed to ____ %s\n", desc);
	exit(1);
}

SDL_Texture *load_texture(const char *filename)
{
	SDL_Surface *loaded_surface = SDL_LoadBMP(filename);
	SDL_SetColorKey(loaded_surface, SDL_TRUE, SDL_MapRGB(loaded_surface->format, 0xff, 0x00, 0xff));
	SDL_Texture *new_texture = SDL_CreateTextureFromSurface(g_renderer, loaded_surface);
	SDL_FreeSurface(loaded_surface);
	return new_texture;
}

void init_sdl()
{
	ensure((SDL_Init(SDL_INIT_EVERYTHING) >= 0), "sdl");

	ensure((g_window = SDL_CreateWindow("Slasher Escape", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN)), "window");
	g_width = WINDOW_W / g_scale;
	g_height = WINDOW_H / g_scale;

	ensure((g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE)), "renderer");
	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);

	ensure((g_tileset = load_texture("data/tileset.bmp")), "tileset");
	ensure((g_font = load_texture("data/font.bmp")), "font");
	ensure((g_vigenette = load_texture("data/vigenette.bmp")), "vigenette");
	ensure((g_gameover = load_texture("data/game_over.bmp")), "game over");
}

void end_sdl()
{
	SDL_DestroyTexture(g_gameover);
	g_gameover = NULL;
	SDL_DestroyTexture(g_vigenette);
	g_vigenette = NULL;
	SDL_DestroyTexture(g_font);
	g_font = NULL;
	SDL_DestroyTexture(g_tileset);
	g_tileset = NULL;

	SDL_DestroyRenderer(g_renderer);
	g_renderer = NULL;
	SDL_DestroyWindow(g_window);
	g_window = NULL;
	SDL_Quit();
}

void draw_texture_region(SDL_Texture *texture, int cx, int cy, int cw, int ch, int dx, int dy, int dw, int dh)
{
	SDL_Rect clip, dest;
	clip.x = cx;
	clip.y = cy;
	clip.w = cw;
	clip.h = ch;
	dest.x = dx * g_scale;
	dest.y = dy * g_scale;
	dest.w = dw * g_scale;
	dest.h = dh * g_scale;

	SDL_RenderCopy(g_renderer, texture, &clip, &dest);
}

void draw_pixel(int x, int y)
{
	SDL_Rect r;
	r.x = x * g_scale;
	r.y = y * g_scale;
	r.w = g_scale;
	r.h = g_scale;
	SDL_RenderFillRect(g_renderer, &r);
}

void draw_box(int x, int y, int w, int h)
{
	SDL_Rect r;
	r.x = x * g_scale;
	r.y = y * g_scale;
	r.w = w * g_scale;
	r.h = h * g_scale;
	SDL_RenderFillRect(g_renderer, &r);
}

void save_map(const char *filename)
{
	FILE *fp = fopen(filename, "w");
	fprintf(fp, "%d %d\n", g_map_w, g_map_h);
	for(int y = 0; y < g_map_h; y++)
	{
		for(int x = 0; x < g_map_w; x++)
			fprintf(fp, "%d ", g_map[y*g_map_w+x]);
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void new_map(int w, int h)
{
	g_map_w = w;
	g_map_h = h;
	g_map = malloc(sizeof(int)*w*h);
	for(int i = 0; i < g_map_w*g_map_h; i++)
		g_map[i] = 0;
}

void load_map(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	fscanf(fp, "%d %d", &g_map_w, &g_map_h);
	g_map = malloc(sizeof(int)*g_map_w*g_map_h);
	for(int i = 0; i < g_map_w*g_map_h; i++)
		fscanf(fp, "%d", &g_map[i]);
	fclose(fp);
}

void draw_text(int x, int y, const char *text)
{
	int tx = x, ty = y;
	for(int i = 0; text[i] != 0; i++)
	{
		char c = text[i];
		if(c == '\n')
		{
			tx = x;
			ty += 6;
			continue;
		}
		if(c == ' ')
		{
			tx += 4;
			continue;
		}

		if(c >= 0x30 && c <= 0x39)
			c = c - 0x30 + 30;
		else if(c >= 0x41 && c <= 0x5a)
			c -= 0x41;
		else if(c >= 0x61 && c <= 0x7a)
			c -= 0x61;
		else if(c == ':')
			c = 29;
		else if(c == '\'')
			c = 28;
		else if(c == '+')
			c = 27;
		else if(c == '.')
			c = 26;
		else
			continue;

		int cx = (c % 10) * 4;
		int cy = (c / 10) * 6;
		draw_texture_region(g_font, cx, cy, 4, 6, tx, ty, 4, 6);
		tx += 4;
	}
}

void init_map()
{
	FILE *fp = fopen("data/level.lvl", "r");
	if(fp)
	{
		fclose(fp);
		load_map("data/level.lvl");
	}
	else
		new_map(150, 100);
}

void toggle_fullscreen()
{
	g_fullscreen = !g_fullscreen;
	Uint32 flags;
	g_fullscreen ? (flags = SDL_WINDOW_FULLSCREEN_DESKTOP) : (flags = SDL_WINDOW_SHOWN);
	SDL_SetWindowSize(g_window, WINDOW_W, WINDOW_H);
	g_scale = 4;
	g_width = WINDOW_W / g_scale;
	g_height = WINDOW_H / g_scale;
	SDL_SetWindowFullscreen(g_window, flags);
	SDL_DisplayMode dm;
	if(g_fullscreen)
	{
		SDL_GetCurrentDisplayMode(0, &dm);
		g_scale = (((dm.h/160)>>1)+1)<<1;
		g_width = dm.w / g_scale;
		g_height = dm.h / g_scale;
	}
	SDL_ShowCursor(!g_fullscreen);
}

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
					if(editor_tile > 7)
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

void game_over()
{
	SDL_Event event;
	int last_update = SDL_GetTicks();
	bool quit = false, redraw = true;
	int frame = 0, stage = 0;
	int max_frames[4] = {12, 20, 25, 50};

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
				case SDLK_z:
				case SDLK_x:
				case SDLK_RETURN:
					if(stage == 3)
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
				draw_texture_region(g_gameover, 80, 0, 40, 60, g_width/2-20, g_height/2-30-mul*15, 40, 60);
				draw_texture_region(g_gameover, 80, 60, 40, 60, g_width/2-20, g_height-60+2+mul*10, 40, 60);
				break;
			case 3:
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
			if(stage < 3)
			{
				frame++;
				if(frame > max_frames[stage])
				{
					stage++;
					frame = 0;
					if(stage == 2)
						SDL_SetRenderDrawColor(g_renderer, 0xff, 0x00, 0x00, 0xff);
				}
			}

			redraw = true;

			last_update = current_time;
		}
	}

	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
}

void init_actor(struct actor *a, int x, int y, int skin)
{
	a->x = x*8;
	a->y = y*8;
	a->xv = 0;
	a->yv = 0;
	a->skin = skin;
	a->d = 0;
	a->speed = 1;
	a->near_open_door = false;
	a->near_closed_door = false;
	a->near_closed_cabinet = false;
	a->near_window = false;
	a->near_item = -1;
}

void draw_actor(struct actor a, int xo, int yo)
{
	bool step;
	a.xv != 0 ? (step = ((int)a.x/8) % 2) : (step = ((int)a.y/8) % 2);
	draw_texture_region(g_tileset, a.skin*16 + step*8, 8*8 + a.d*8, 8, 8, a.x-xo, a.y-yo, 8, 8);
}

void spawn_actor(struct actor *a, int skin)
{
	int ix = rand () % (g_map_w-30);
	int iy = rand () % (g_map_h-30);
	for(int x = ix; x < ix+30; x++)
		for(int y = iy; y < iy+30; y++)
			if(g_map[y*g_map_w+x] == 0)
			{
				init_actor(a, x, y, skin);
				return;
			}
}

bool within_tile(int x, int y, int tx, int ty)
{
	return (x > tx*8 && y > ty*8 && x < tx*8+8 && y < ty*8+8);
}

bool move_actor(struct actor *a, float x, float y)
{
	float dx = a->x+x, dy = a->y+y;
	int x1 = dx+2, y1 = dy+1, x2 = dx+6, y2 = dy+8;

	if(x1 < 0 || y1 < 0 || x2 >= g_map_w*8 || y2 >= g_map_h*8)
		return false;
	for(int tx = x1/8-1; tx <= x2/8+1; tx++)
		for(int ty = y1/8-1; ty <= y2/8+1; ty++)
		{
			if(tx < 0 || ty < 0 || tx >= g_map_w || ty >= g_map_h)
				continue;
			int t = g_map[ty*g_map_w+tx];
			if(t==2||t==3||t==5||t==6||t==7||t==11)
				if(within_tile(x1, y1, tx, ty) || within_tile(x2, y1, tx, ty) || within_tile(x1, y2, tx, ty) || within_tile(x2, y2, tx, ty))
					return false;
		}

	a->x = dx;
	a->y = dy;
	return true;
}

bool move_actor_and_slide(struct actor *a, float sa, float v)
{
	for(float am = 0; am < PI*0.4; am += 0.01)
	{
		if(move_actor(a, cosf(sa+am)*v, sinf(sa+am)*v)) return true;
		if(move_actor(a, cosf(sa-am)*v, sinf(sa-am)*v)) return true;
	}
	return false;
}

int tile_at(int x, int y)
{
	return g_map[(y/8)*g_map_w+(x/8)];
}

void update_actor(struct actor *a)
{
	if(a->xv != 0 || a->yv != 0)
	{
		float ma = atan2(a->yv, a->xv);
		float i;
		for(i = a->speed; i > 0; i -= 0.05)
			if(move_actor_and_slide(a, ma, i)) break;
		if(i > 0)
		{
			if(a->yv > 0) a->d = 0;
			else if(a->yv < 0) a->d = 2;
			if(a->xv > 0) a->d = 1;
			else if(a->xv < 0) a->d = 3;
		}
	}

	a->near_open_door = false;
	a->near_closed_door = false;
	a->near_closed_cabinet = false;
	a->near_window = false;
	int offsets[8] = {
		0, 1,
		1, 0,
		0, -1,
		-1, 0,
	};
	for(int i = 0; i < 4; i++)
	{
		int x = a->x+4+offsets[i*2]*8;
		int y = a->y+4+offsets[i*2+1]*8;
		int t = tile_at(x, y);
		bool near = true;
		if(t == 6)
			a->near_window = true;
		if(t == 7)
			a->near_closed_cabinet = true;
		else if(t == 5)
			a->near_closed_door = true;
		else if(t == 9)
			a->near_open_door = true;
		else
			near = false;
		if(near) break;
	}

	a->near_item = -1;
	int i;
	for(i = 0; g_items[i].type != -1; i++)
		if(pow(g_items[i].x-a->x-4, 2) + pow(g_items[i].y-a->y-4, 2) < 8*8)
			break;
	if(g_items[i].type != -1) a->near_item = i;
}

void actor_smash(struct actor a)
{
	int offsets[8] = {
		0, 1,
		1, 0,
		0, -1,
		-1, 0,
	};
	for(int i = 0; i < 4; i++)
	{
		if((a.xv == offsets[i*2]*-1 && a.xv != 0)||(a.yv == offsets[i*2+1]*-1 && a.yv != 0))
			continue;
		int x = a.x+4+offsets[i*2]*8;
		int y = a.y+4+offsets[i*2+1]*8;
		int *t = &g_map[(y/8)*g_map_w+(x/8)];
		if(*t == 6)
			*t = 10;
		else if(*t == 5)
			*t = 8;
	}
}

void teleport_actor_near(struct actor *a, int tx, int ty)
{
	int x, y;
	while(1)
	{
		float a = (rand() % (int)(PI*1000))/1000;
		int dist = 55 + rand() % 20;
		x = (tx+cosf(a)*dist)/8;
		y = (ty+sinf(a)*dist)/8;
		int t = g_map[y*g_map_w+x];
		if(t == 0 || t == 4)
			break;
	}
	a->x = x*8;
	a->y = y*8;
}

void actor_interact_tile(struct actor a)
{
	int offsets[8] = {
		0, 1,
		1, 0,
		0, -1,
		-1, 0,
	};
	for(int i = 0; i < 4; i++)
	{
		int x = a.x + 4 + offsets[i*2]*8;
		int y = a.y + 4 + offsets[i*2+1]*8;
		int *t = &g_map[(y/8)*g_map_w+(x/8)];
		bool interacted = true;
		if(*t == 7)
			*t = 11;
		else if(*t == 5)
			*t = 9;
		else if(*t == 9)
			*t = 5;
		else
			interacted = false;
		if(interacted)
			break;
	}
}

void add_item(int x, int y, int type)
{
	int i;
	for(i = 0; g_items[i].type != -1; i++);
	g_items[i].x = x;
	g_items[i].y = y;
	g_items[i].xv = 0;
	g_items[i].yv = 0;
	g_items[i].type = type;
}

void draw_item(struct item i, int xo, int yo)
{
	draw_texture_region(g_tileset, i.type*8, 3*8, 8, 8, i.x-4-xo, i.y-4-yo, 8, 8);
}

void delete_item(struct item *it)
{
	int i, j;
	for(i = 0; &g_items[i] != it; i++);
	for(j = 0; g_items[j+1].type != -1; j++);
	struct item *s = &g_items[j];
	struct item *d = &g_items[i];
	d->x = s->x;
	d->y = s->y;
	d->type = s->type;
	d->xv = s->xv;
	d->yv = s->yv;
	s->type = -1;
}

int projectile_item_at(int x, int y)
{
	for(int i = 0; g_items[i].type != -1; i++)
	{
		struct item *it = &g_items[i];
		if(fabs(it->xv) < 1 && fabs(it->yv) < 1)
			continue;
		if(pow(x-it->x, 2) + pow(y-it->y, 2) < 4*4)
			return i;
	}
	return -1;
}

bool update_item(struct item *it)
{
	if(it->xv == 0 && it->yv == 0)
		return false;
	it->x += it->xv;
	it->y += it->yv;
	int t = tile_at(it->x, it->y);
	if(t==2||t==3||(t>=5&&t<=7)||t==11)
	{
		it->x -= it->xv;
		it->y -= it->yv;
		it->xv *= -1;
		it->yv *= -1;
	}
	if(it->xv > 0) it->xv -= 0.5;
	if(it->yv > 0) it->yv -= 0.5;
	if(it->xv < 0) it->xv += 0.5;
	if(it->yv < 0) it->yv += 0.5;
	return true;
}

void actor_throw_item(struct actor *a, int type)
{
	int vels[8] = {
		0, 1,
		1, 0,
		0, -1,
		-1, 0,
	};
	int i;
	for(i = 0; g_items[i].type != -1; i++);
	int xv = vels[a->d*2]*4;
	int yv = vels[a->d*2+1]*4;
	add_item(a->x+4+xv, a->y+4+yv, type);
	g_items[i].xv = xv;
	g_items[i].yv = yv;
}

void play_game()
{
	SDL_Event event;
	struct actor player, killer;
	init_map();
	spawn_actor(&player, 0);
	init_actor(&killer, 0, 0, 1);
	teleport_actor_near(&killer, player.x+4, player.y+4);
	bool quit = false, redraw = true, killer_chasing = false, player_full = false;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	int last_update = SDL_GetTicks(), camera_x = player.x + 4, camera_y = player.y + 4;
	int killer_chase_counter = 200, cabinet_count = 0, items_left = 5, player_selected = 0, throw_charge = 0;
	int player_items[3] = {-1, -1, -1};
	for(int i = 0; i < MAX_STUFF; i++)
		g_items[i].type = -1;
	for(int x = 0; x < g_map_w; x++)
		for(int y = 0; y < g_map_h; y++)
			if(g_map[y*g_width+x] == 7) cabinet_count++;

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
					if(player.near_item != -1 && !player_full)
					{
						int i;
						for(i = 0; i < 3; i++)
							if(player_items[i] == -1) break;
						player_items[i] = g_items[player.near_item].type;
						delete_item(&g_items[player.near_item]);
						for(i = 0; i < 3; i++)
							if(player_items[i] == -1) break;
						if(i >= 3) player_full = true;
						redraw = true;
						break;
					}
					if(player.near_open_door||player.near_closed_door||player.near_closed_cabinet)
					{
						if(player.near_closed_cabinet)
						{
							if(rand() % cabinet_count >= cabinet_count - items_left && items_left > 0)
							{
								int t = 5 - items_left;
								add_item(player.x+4, player.y+4, t);
								items_left--;
							}
							cabinet_count--;
						}
						actor_interact_tile(player);
						redraw = true;
					}
					break;
				case SDLK_x:
					if(throw_charge > 5)
					{
						throw_charge = 0;
						break;
					}
					player_selected++;
					if(player_selected >= 3)
						player_selected = 0;
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
			bool items_moved = false;
			for(int i = 0; g_items[i].type != -1; i++)
				if(update_item(&g_items[i]))  items_moved = true;
			if(items_moved)
				redraw = true;

			if(keyboard_state[SDL_SCANCODE_X] && player_items[player_selected] != -1)
			{
				throw_charge++;
				if(throw_charge > 5)
				{
					actor_throw_item(&player, player_items[player_selected]);
					player_items[player_selected] = -1;
					redraw = true;
				}
			}

			if(killer_chase_counter > 0)
				killer_chase_counter--;
			else
			{
				killer_chasing = !killer_chasing;
				if(killer_chasing)
					killer_chase_counter = 150 + rand() % 75;
				else
					killer_chase_counter = 100 - rand() % 50;
			}

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
			if((killer.near_window||killer.near_closed_door) && killer_chasing)
			{
				actor_smash(killer);
				redraw = true;
			}

			if(tile_at(player.x+4, player.y+4) == 4)
				player.speed = 1;
			else
				player.speed = 1.5;
			update_actor(&player);
			if(player.xv != 0 || player.yv != 0)
				redraw = true;

			if(tile_at(killer.x+4, killer.y+4) == 4)
				killer.speed = 1.75;
			else
				killer.speed = 1.25;
			if(!killer_chasing)
				killer.speed *= 0.5;
			update_actor(&killer);
			if(pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) < 50*50)
				redraw = true;
			else if(pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) > 160*160 && killer_chasing)
				teleport_actor_near(&killer, player.x, player.y);

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

			if(pow(player.x-killer.x, 2) + pow(player.y-killer.y, 2) < 7*7)
			{
				game_over();
				quit = true;
				redraw = false;
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
					draw_actor(killer, xo, yo);
				draw_actor(player, xo, yo);
				if(player_items[player_selected] != -1)
				{
					int t = player_items[player_selected];
					draw_texture_region(g_tileset, (t % 4)*8, 3*8+(t / 4)*8, 8, 8, player.x-xo, player.y-yo, 8, 8);
				}
				if(player.near_item != -1 && !player_full)
					draw_text(player.x-xo-4.5*4, player.y-yo-6-1, "z:take item");
				else if(player.near_open_door)
					draw_text(player.x-xo-5*4, player.y-yo-6-1, "z:close door");
				else if(player.near_closed_door)
					draw_text(player.x-xo-5.5*4, player.y-yo-6-1, "z:open door");
				else if(player.near_closed_cabinet)
					draw_text(player.x-xo-4*4, player.y-yo-6-1, "z:search");
				draw_texture_region(g_vigenette, 0, 0, 160, 120, player.x+4-80-xo, player.y+4-60-yo, 160, 120);
				for(int i = 0; i < 3; i++)
				{
					SDL_SetRenderDrawColor(g_renderer, 0x38, 0x50, 0x20, 0xff);
					draw_box(1+i*9, 1, 8, 8);
					SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
					int t = player_items[i];
					if(t != -1)
					{
						draw_texture_region(g_tileset, (t % 4)*8, 3*8+(t / 4)*8, 8, 8, 1+i*9, 1, 8, 8);
					}
				}
				draw_texture_region(g_tileset, 2*8, 4*8, 8, 8, 1+player_selected*9, 1+8, 8, 8);
				SDL_RenderPresent(g_renderer);

				redraw = false;
			}

			last_update = current_time;
		}

	}

	free(g_map);
}

void main_menu()
{
	SDL_Event event;
	bool quit = false, redraw = true, confirm = false;
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
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_UP:
					if(cursor_yo == 0 && cursor_y > 0)
						cursor_yo = -1;
					break;
				case SDLK_DOWN:
					if(cursor_yo == 0 && cursor_y < 2)
						cursor_yo = 1;
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
			draw_text(g_width/2 - 7*4, 6, "slasher escape");
			draw_text(g_width/2 - 4*4, g_height/2-2*6, "new game\neditor\nquit");
			draw_texture_region(g_tileset, (bool)(cursor_step / 10)*8, 8*8, 8, 8, g_width/2-8*4, g_height/2-2*6 + cursor_y*6+cursor_yo-1, 8, 8);
			if(!g_fullscreen)
				draw_text(g_width-20*4, g_height-6, "alt+enter:fullscreen");
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
			switch(cursor_y)
			{
			case 0:
				play_game();
				break;
			case 1:
				editor();
				break;
			case 2:
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
	SDL_FlushEvent(SDL_KEYDOWN);
}

int main()
{
	time_t t;
	srand(time(&t));
	init_sdl();
	play_intro();
	main_menu();
	end_sdl();
	return 0;
}
