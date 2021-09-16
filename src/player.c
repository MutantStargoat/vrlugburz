#include "player.h"

void init_player(struct player *p)
{
	memset(p, 0, sizeof *p);
	cgm_qcons(&p->vrot, 0, 0, 0, 1);

	p->height = 1.75;
	p->hp = p->hp_max = 10;
	p->mp = p->mp_max = 10;
}

void upd_player_xform(struct player *p)
{
	cgm_vec3 pos;
	float celld = p->lvl ? p->lvl->cell_size : DEF_CELL_SIZE;

	cgm_vcons(&pos, p->cx * celld, p->height, -p->cy * celld);
	cgm_vadd(&pos, &p->cpos);

	cgm_midentity(p->view_xform);
	cgm_mprerotate_x(p->view_xform, -p->phi);
	cgm_mprerotate_y(p->view_xform, -p->theta);
	cgm_mrotate_quat(p->view_xform, &p->vrot);
	cgm_mpretranslate(p->view_xform, -pos.x, -pos.y, -pos.z);
}
