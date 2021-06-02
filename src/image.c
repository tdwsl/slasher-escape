#include <SDL2/SDL.h>
#include <stdbool.h>
#include "ensure.h"
#include "image.h"

#define WINDOW_W 640
#define WINDOW_H 480

SDL_Renderer *g_renderer = NULL;
SDL_Window *g_window = NULL;
SDL_Texture *g_tileset = NULL, *g_font = NULL, *g_vigenette = NULL, *g_gameover = NULL, *g_arrow = NULL, *g_victory = NULL;
int g_scale = 4, g_width = 160, g_height = 120;
bool g_fullscreen = false;

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
	ensure((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) >= 0), "sdl");

	ensure((g_window = SDL_CreateWindow("Slasher Escape", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN)), "window");
	g_width = WINDOW_W / g_scale;
	g_height = WINDOW_H / g_scale;

	ensure((g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE)), "renderer");
	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);

	SDL_Surface *icon = NULL;
	ensure((icon = SDL_LoadBMP("data/img/icon.bmp")), "icon");
	SDL_SetWindowIcon(g_window, icon);
	SDL_FreeSurface(icon);
	icon = NULL;

	ensure((g_tileset = load_texture("data/img/tileset.bmp")), "tileset");
	ensure((g_font = load_texture("data/img/font.bmp")), "font");
	ensure((g_vigenette = load_texture("data/img/vigenette.bmp")), "vigenette");
	ensure((g_gameover = load_texture("data/img/game_over.bmp")), "game over");
	ensure((g_arrow = load_texture("data/img/arrow.bmp")), "arrow texture");
	ensure((g_victory = load_texture("data/img/victory.bmp")), "victory texture");
}

void end_sdl()
{
	SDL_DestroyTexture(g_victory);
	g_victory = NULL;
	SDL_DestroyTexture(g_arrow);
	g_arrow = NULL;
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
		int hs = ((dm.h/120)>>1)<<1;
		int ws = ((dm.w/160)>>1)<<1;
		(ws < hs) ? (g_scale = ws) : (g_scale = hs);
		g_width = dm.w / g_scale;
		g_height = dm.h / g_scale;
	}
	SDL_ShowCursor(!g_fullscreen);
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
		else if(c == '/')
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
