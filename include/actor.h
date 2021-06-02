#ifndef SLASHER_ACTOR
#define SLASHER_ACTOR

#include <stdbool.h>

struct actor
{
	int xv, yv, skin, d, near_item;
	float x, y, speed;
	bool near_open_door, near_closed_door, near_closed_cabinet, near_window, near_tree;
};

void init_actor(struct actor *a, int x, int y, int skin);
void draw_actor(struct actor a, int xo, int yo);
void spawn_actor(struct actor *a, int skin);
bool move_actor(struct actor *a, float x, float y);
bool move_actor_and_slide(struct actor *a, float sa, float v);
void update_actor(struct actor *a);
void actor_smash(struct actor a);
void teleport_actor_near(struct actor *a, int tx, int ty);
void actor_interact_tile(struct actor a);
void actor_throw_item(struct actor *a, int type);
void actor_chop(struct actor a);
bool actor_shoot(struct actor a, bool destroys, int dx, int dy);

#endif
