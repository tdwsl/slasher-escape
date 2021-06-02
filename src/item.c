#include <stdbool.h>
#include <math.h>
#include "image.h"
#include "item.h"
#include "map.h"
#include "audio.h"
#include "game.h"

struct item g_items[MAX_STUFF];

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
	draw_texture_region(g_tileset, (i.type % 4)*8, 4*8+(i.type / 4)*8, 8, 8, i.x-4-xo, i.y-4-yo, 8, 8);
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
	if(t==6)
	{
		g_map[(int)(it->y/8)*g_map_w+(int)(it->x/8)] = 12;
		play_audio(g_sfx_chop);
	}
	else if((t==8||t==9) && it->type % 2 == 0)
	{
		g_car_parts++;
		play_audio(g_sfx_item);
		return true;
	}
	else if(t==2||t==3||t==5||t==7||t==13||t==8||t==9)
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
