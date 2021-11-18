#include "psysman.h"
#include "darray.h"

void psman_init(struct psysman *pm)
{
	pm->last_empty_psys = pm->last_empty_tmp = -1;
}

void psman_destroy(struct psysman *pm)
{
	psman_clear(pm);
	darr_free(pm->psys);
	darr_free(pm->psystmp);
}

void psman_clear(struct psysman *pm)
{
	int i, num;

	num = darr_size(pm->psystmp);
	for(i=0; i<num; i++) {
		psys_free(pm->psystmp[i]);
	}

	darr_clear(pm->psys);
	darr_clear(pm->psystmp);
	pm->last_empty_psys = pm->last_empty_tmp = -1;
}

void psman_add_emitter(struct psysman *pm, struct psys_emitter *ps)
{
	int i, num;

	if(pm->last_empty_psys >= 0) {
		pm->psys[pm->last_empty_psys] = ps;
		pm->last_empty_psys = -1;
		return;
	}

	num = darr_size(pm->psys);
	for(i=0; i<num; i++) {
		if(!pm->psys[i]) {
			pm->psys[i] = ps;
			return;
		}
	}

	darr_push(pm->psys, &ps);
}

void psman_add_tmp_emitter(struct psysman *pm, struct psys_emitter *ps)
{
	int i, num;

	if(pm->last_empty_tmp >= 0) {
		pm->psystmp[pm->last_empty_tmp] = ps;
		pm->last_empty_tmp = -1;
		return;
	}

	num = darr_size(pm->psystmp);
	for(i=0; i<num; i++) {
		if(!pm->psystmp[i]) {
			pm->psystmp[i] = ps;
			return;
		}
	}

	darr_push(pm->psystmp, &ps);
}

void psman_remove(struct psysman *pm, struct psys_emitter *ps)
{
	int i, num;

	num = darr_size(pm->psys);
	for(i=0; i<num; i++) {
		if(pm->psys[i] == ps) {
			pm->psys[i] = 0;
			pm->last_empty_psys = i;
			return;
		}
	}

	num = darr_size(pm->psystmp);
	for(i=0; i<num; i++) {
		if(pm->psystmp[i] == ps) {
			psys_free(ps);
			pm->psystmp[i] = 0;
			pm->last_empty_tmp = i;
			return;
		}
	}
}

void psman_update(struct psysman *pm, long tm)
{
	int i, num;
	struct psys_emitter *ps;

	num = darr_size(pm->psys);
	for(i=0; i<num; i++) {
		if(pm->psys[i]) {
			psys_update(pm->psys[i], tm);
		}
	}

	num = darr_size(pm->psystmp);
	for(i=0; i<num; i++) {
		if(!(ps = pm->psystmp[i])) continue;
		psys_update(ps, tm);
		if(!ps->pcount && psys_get_cur_value(&ps->attr.rate) <= 0.0f) {
			/* stopped spawning, remove emitter */
			psys_free(ps);
			pm->psystmp[i] = 0;
			pm->last_empty_tmp = i;
		}
	}
}

void psman_draw(struct psysman *pm)
{
	int i, num;

	num = darr_size(pm->psys);
	for(i=0; i<num; i++) {
		if(pm->psys[i]) {
			psys_draw(pm->psys[i]);
		}
	}

	num = darr_size(pm->psystmp);
	for(i=0; i<num; i++) {
		if(pm->psystmp[i]) {
			psys_draw(pm->psystmp[i]);
		}
	}
}
