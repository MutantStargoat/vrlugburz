#ifndef PLAYER_H_
#define PLAYER_H_

#include "cgmath/cgmath.h"
#include "level.h"

struct player {
	struct level *lvl;

	int cx, cy;
	cgm_vec3 cpos;		/* cell position (derived from cx,cy) */
	float theta, phi;	/* mouselook/VR controller rotation (theta only) */
	cgm_vec3 vpos;		/* VR position within the cell */
	cgm_quat vrot;		/* VR orientation */

	int dir;			/* cardinal direction, clockwise, 0 is north */
	float height;

	/* view matrix, derived from all of the above by upd_player_xform */
	float view_xform[16];

	int hp, mp, hp_max, mp_max;
};

void init_player(struct player *p);

void update_player_dir(struct player *p);
void move_player(struct player *p, int right, int fwd);
void turn_player(struct player *p, int turn);

void upd_player_xform(struct player *p);

int cell_rank(struct player *p, int x, int y);
int cell_infront(struct player *p, int x, int y);

#endif	/* PLAYER_H_ */
