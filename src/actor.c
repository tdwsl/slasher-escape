#include <stdbool.h>
#include <math.h>
#include "image.h"
#include "actor.h"
#include "map.h"
#include "game.h"
#include "audio.h"
#include "item.h"

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
	a->near_tree = false;
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

bool move_actor(struct actor *a, float x, float y)
{
	float dx = a->x+x, dy = a->y+y;
	int x1 = dx+2, y1 = dy+1, x2 = dx+6, y2 = dy+8;

	if(x1 < 0 || y1 < 0 || x2 >= g_map_w*8 || y2 >= g_map_h*8)
		return false;
	
	int offsets[8] = {
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};
	for(int i = 0; i < 4; i++)
	{
		int tx = dx+2 + offsets[i*2]*4;
		int ty = dy+1 + offsets[i*2+1]*7;
		if(tx < 0 || ty < 0 || tx >= g_map_w*8 || ty >= g_map_h*8)
			return false;
		int t = tile_at(tx, ty);
		if(t==2||t==3||t==5||t==6||t==7||t==13||t==8||t==9)
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
	a->near_tree = false;
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
		if(t == 6)
			a->near_window = true;
		if(t == 7)
			a->near_closed_cabinet = true;
		if(t == 5)
			a->near_closed_door = true;
		if(t == 11)
			a->near_open_door = true;
		if(t == 3 && i == a->d)
			a->near_tree = true;
	}
	if(a->near_closed_cabinet)
	{
		a->near_closed_door = false;
		a->near_open_door = false;
	}
	if(a->near_closed_door)
		a->near_open_door = false;

	a->near_item = -1;
	int i;
	for(i = 0; g_items[i].type != -1; i++)
	{
		struct item *it = &g_items[i];
		if(pow(it->x-a->x-4, 2) + pow(it->y-a->y-4, 2) < 8*8 && it->xv == 0 && it->yv == 0)
			break;
	}
	if(g_items[i].type != -1) a->near_item = i;
}

void actor_smash(struct actor a)
{
	int offsets[10] = {
		0, 0,
		0, 1,
		1, 0,
		0, -1,
		-1, 0,
	};
	for(int i = 0; i < 5; i++)
	{
		if((a.xv == offsets[i*2]*-1 && a.xv != 0)||(a.yv == offsets[i*2+1]*-1 && a.yv != 0))
			continue;
		int x = a.x+4+offsets[i*2]*8;
		int y = a.y+4+offsets[i*2+1]*8;
		int *t = &g_map[(y/8)*g_map_w+(x/8)];
		bool smashed = false;
		if(*t == 6)
		{
			*t = 12;
			smashed = true;
		}
		else if(*t == 5)
		{
			*t = 10;
			smashed = true;
		}
		if(smashed)
		{
			play_audio(g_sfx_chop);
			return;
		}
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
		if(*t == 7 && a.near_closed_cabinet)
			*t = 13;
		else if(*t == 5 && a.near_closed_door)
			*t = 11;
		else if(*t == 11 && a.near_open_door)
			*t = 5;
		else
			interacted = false;
		if(interacted)
		{
			play_audio(g_sfx_open);
			break;
		}
	}
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

void actor_chop(struct actor a)
{
	int offsets[8] = {
		0, 1,
		1, 0,
		0, -1,
		-1, 0,
	};
	int x = a.x+4+offsets[a.d*2]*8;
	int y = a.y+4+offsets[a.d*2+1]*8;
	int *t = &g_map[(y/8)*g_map_w+(x/8)];
	if(*t == 3)
		*t = 14;
}

bool actor_shoot(struct actor a, bool destroys, int dx, int dy)
{
	int x = a.x+4;
	int y = a.y+4;
	
	int xv = a.xv/a.speed;
	int yv = a.yv/a.speed;
	if(xv == 0 && yv == 0)
	{
		int vels[8] = {
			0, 1,
			1, 0,
			0, -1,
			-1, 0,
		};
		xv = vels[a.d*2];
		yv = vels[a.d*2+1];
	}
	
	for(int i = 0; i < 50; i++)
	{
		x += xv;
		y += yv;
		int *t = &g_map[(y/8)*g_map_w+(x/8)];
		if(*t==2||*t==3||*t==8||*t==9)
			return false;
		if(*t==5||*t==6)
		{
			if(!destroys)
				return false;
			if(*t==5)
				*t = 10;
			if(*t==6)
				*t = 12;
			play_audio(g_sfx_chop);
		}
		if(pow(dx-x, 2) + pow(dy-y, 2) < 8*8)
			return true;
	}
	return false;
}
