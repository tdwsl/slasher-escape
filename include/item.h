#ifndef SLASHER_ITEMS
#define SLASHER_ITEMS

#include <stdbool.h>

#define MAX_STUFF 25

struct item
{
	int type;
	float x, y, xv, yv;
};

extern struct item g_items[MAX_STUFF];

void add_item(int x, int y, int type);
void draw_item(struct item i, int xo, int yo);
void delete_item(struct item *it);
int projectile_item_at(int x, int y);
bool update_item(struct item *it);

#endif
