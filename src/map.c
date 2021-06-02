#include <stdlib.h>
#include <stdio.h>
#include "map.h"

int *g_map, g_map_w, g_map_h;

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

int tile_at(int x, int y)
{
	return g_map[(y/8)*g_map_w+(x/8)];
}
