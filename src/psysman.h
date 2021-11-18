#ifndef PSYSMAN_H_
#define PSYSMAN_H_

#include "psys/psys.h"

struct psysman {
	/* persistent particles systems, dynamic array, null on remove, doesn't own */
	struct psys_emitter **psys;
	int last_empty_psys;
	/* ephemeral particle systems, owned by psysman */
	struct psys_emitter **psystmp;
	int last_empty_tmp;
};

void psman_init(struct psysman *pm);
void psman_destroy(struct psysman *pm);

void psman_clear(struct psysman *pm);

void psman_add_emitter(struct psysman *pm, struct psys_emitter *ps);
void psman_add_tmp_emitter(struct psysman *pm, struct psys_emitter *ps);

void psman_remove(struct psysman *pm, struct psys_emitter *ps);

void psman_update(struct psysman *pm, long tm);
void psman_draw(struct psysman *pm);

#endif	/* PSYSMAN_H_ */
