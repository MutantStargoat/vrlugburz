#include <stdio.h>
#include "rend.h"

struct renderer *init_rend_debug(void);
struct renderer *init_rend_level(void);

int rend_init(void)
{
	if(!(rend[REND_DBG] = init_rend_debug())) {
		fprintf(stderr, "failed to create debug renderer\n");
		return -1;
	}
	if(!(rend[REND_LEVEL] = init_rend_level())) {
		fprintf(stderr, "failed to create level renderer\n");
		return -1;
	}

	return 0;
}

void rend_destroy(void)
{
	int i;

	for(i=0; i<NUM_REND; i++) {
		if(rend[i]->destroy) rend[i]->destroy();
	}
}

void rend_reshape(int x, int y)
{
	int i;

	for(i=0; i<NUM_REND; i++) {
		if(rend[i]->reshape) rend[i]->reshape(x, y);
	}
}

void rend_begin(int rid, int pass)
{
	if(rend[rid]->beginpass) rend[rid]->beginpass(pass);
}

void rend_end(int rid, int pass)
{
	if(rend[rid]->endpass) rend[rid]->endpass(pass);
}

void rend_pass(int rid, int pass, struct scene *scn)
{
	if(rend[rid]->rendpass[pass]) {
		rend[rid]->rendpass[pass](scn);
	}
}
