#ifndef SLASHER_MAP
#define SLASHER_MAP

extern int *g_map, g_map_w, g_map_h;

void save_map(const char *filename);
void load_map(const char *filename);
void init_map();
int tile_at(int x, int y);

#endif
