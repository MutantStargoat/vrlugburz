#include "player.h"
#include "util.h"

static const int step[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

void init_player(struct player *p)
{
	memset(p, 0, sizeof *p);
	cgm_qcons(&p->vrot, 0, 0, 0, 1);

	p->height = 1.75;
	p->hp = p->hp_max = 10;
	p->mp = p->mp_max = 10;
}

#define TWO_PI		((float)M_PI * 2.0f)
#define HALF_PI		((float)M_PI / 2.0f)

void update_player_dir(struct player *p)
{
	int prev_dir = p->dir;
	float angle;

	/* TODO: take vrot into account */
	angle = fmod(p->theta, TWO_PI);
	if(angle < 0) angle += TWO_PI;

	p->theta = angle;	/* renormalize theta */
	p->dir = (int)(4.0f * angle / TWO_PI + 0.5) & 3;

	if(p->dir != prev_dir) {
		p->vis = 0;	/* invalidate visibility list */
	}
}

void move_player(struct player *p, int right, int fwd)
{
	int fdir, rdir;

	update_player_dir(p);

	fdir = p->dir & 3;
	rdir = (fdir + 1) & 3;
	p->cx += step[fdir][0] * fwd + step[rdir][0] * right;
	p->cy += step[fdir][1] * fwd + step[rdir][1] * right;

	p->vis = 0;	/* invalidate visibility list */
}

void turn_player(struct player *p, int turn)
{
	if(!turn) return;

	p->theta += turn > 0 ? HALF_PI : -HALF_PI;

	update_player_dir(p);
	p->theta = (float)p->dir * HALF_PI;	/* snap theta */

	p->vis = 0;	/* invalidate visibility list */
}

void upd_player_xform(struct player *p)
{
	cgm_vec3 pos;
	float celld = p->lvl ? p->lvl->cell_size : DEF_CELL_SIZE;

	cgm_vcons(&pos, p->cx * celld, p->height, -p->cy * celld);
	cgm_vadd(&pos, &p->cpos);

	cgm_midentity(p->view_xform);
	cgm_mprerotate_x(p->view_xform, -p->phi);
	cgm_mprerotate_y(p->view_xform, p->theta);
	cgm_mrotate_quat(p->view_xform, &p->vrot);
	cgm_mpretranslate(p->view_xform, -pos.x, -pos.y, -pos.z);
}

static void vis_visit(struct player *p, int cx, int cy, int *cvis)
{
	int i, j, nx, ny, dx, dy;
	struct level *lvl = p->lvl;
	struct cell *cell;

	if(cx < 0 || cx >= lvl->width || cy < 0 || cy >= lvl->height) {
		return;
	}
	cell = lvl->cells + cy * lvl->width + cx;

	/* stop when we encounter a solid cell */
	if(cell->type == CELL_SOLID) {
		return;
	}

	dx = cx - p->cx;
	dy = cy - p->cy;
	/* stop beyond the maximum visibility distance (manhattan) */
	if(abs(dx) > lvl->visdist || abs(dy) > lvl->visdist) {
		return;
	}

	/* dot product */
	if(step[p->dir][0] * dx + step[p->dir][1] * dy < 0) {
		return;	/* cell is behind the player */
	}

	cvis[cy * lvl->width + cx] = 1;		/* mark as visited before recursing */

	/* visit neighboring nodes before adding current cell */
	for(i=0; i<3; i++) {
		ny = cy - 1 + i;
		if(ny < 0) continue;
		if(ny >= lvl->height) break;

		for(j=0; j<3; j++) {
			if(i == 1 && j == 1) continue;
			nx = cx - 1 + j;
			if(nx < 0) continue;
			if(nx >= lvl->width) break;

			if(!cvis[ny * lvl->width + nx]) {
				vis_visit(p, nx, ny, cvis);
			}
		}
	}

	/* then add this cell to the visible list */
	cell->next = p->vis;
	p->vis = cell;
}

void upd_player_vis(struct player *p)
{
	int *cvis;
	struct level *lvl = p->lvl;

	cvis = alloca(lvl->width * lvl->height * sizeof *cvis);
	memset(cvis, 0, lvl->width * lvl->height * sizeof *cvis);

	p->vis = 0;
	vis_visit(p, p->cx, p->cy, cvis);
}
