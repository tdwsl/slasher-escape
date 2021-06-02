#ifndef SLASHER_IMAGE
#define SLASHER_IMAGE

#include <SDL2/SDL.h>

extern SDL_Renderer *g_renderer;
extern SDL_Window *g_window;
extern SDL_Texture *g_tileset, *g_font, *g_vigenette, *g_gameover, *g_arrow, *g_victory;
extern int g_scale, g_width, g_height;
extern bool g_fullscreen;

void init_sdl();
void end_sdl();
void draw_texture_region(SDL_Texture *texture, int cx, int cy, int cw, int ch, int dx, int dy, int dw, int dh);
void draw_pixel(int x, int y);
void draw_box(int x, int y, int w, int h);
void toggle_fullscreen();
void draw_text(int x, int y, const char *text);

#endif
