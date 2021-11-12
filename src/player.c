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
	float angle;

	/* TODO: take vrot into account */
	angle = fmod(p->theta, TWO_PI);
	if(angle < 0) angle += TWO_PI;

	p->theta = angle;	/* renormalize theta */
	p->dir = (int)(4.0f * angle / TWO_PI + 0.5) & 3;
}

void move_player(struct player *p, int right, int fwd)
{
	int fdir, rdir;

	update_player_dir(p);

	fdir = p->dir & 3;
	rdir = (fdir + 1) & 3;
	p->cx += step[fdir][0] * fwd + step[rdir][0] * right;
	p->cy += step[fdir][1] * fwd + step[rdir][1] * right;
}

void turn_player(struct player *p, int turn)
{
	if(!turn) return;

	p->theta += turn > 0 ? HALF_PI : -HALF_PI;

	update_player_dir(p);
	p->theta = (float)p->dir * HALF_PI;	/* snap theta */
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
