#include "player.h"

void init_player(struct player *p)
{
	memset(p, 0, sizeof *p);
	cgm_qcons(&p->vrot, 0, 0, 0, 1);

	p->height = 1.75;
	p->hp = p->hp_max = 10;
	p->mp = p->mp_max = 10;
}

void move_player(struct player *p, int right, int fwd)
{
	static const int step[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
	int fdir, rdir;
	float angle;
	cgm_vec3 vdir = {0, 0, -1};

	cgm_vmul_m3v3(&vdir, p->view_xform);

	angle = atan2(vdir.z, vdir.x) + 3.0 * M_PI;
	fdir = (p->dir + (int)(2.0 * angle / M_PI)) & 3;

	rdir = (fdir + 1) & 3;
	p->cx += step[fdir][0] * fwd + step[rdir][0] * right;
	p->cy += step[fdir][1] * fwd + step[rdir][1] * right;
}

void turn_player(struct player *p, int turn)
{
	if(!turn) return;
	turn = turn > 0 ? 1 : 3;
	p->dir = (p->dir + turn) & 3;
	p->theta = 0;
}

void upd_player_xform(struct player *p)
{
	cgm_vec3 pos;
	float celld = p->lvl ? p->lvl->cell_size : DEF_CELL_SIZE;

	cgm_vcons(&pos, p->cx * celld, p->height, -p->cy * celld);
	cgm_vadd(&pos, &p->cpos);

	cgm_midentity(p->view_xform);
	cgm_mprerotate_x(p->view_xform, -p->phi);
	cgm_mprerotate_y(p->view_xform, p->theta + p->dir * M_PI / 2.0f);
	cgm_mrotate_quat(p->view_xform, &p->vrot);
	cgm_mpretranslate(p->view_xform, -pos.x, -pos.y, -pos.z);
}

void upd_player_vis(struct player *p)
{
	p->vis = 0;


}
