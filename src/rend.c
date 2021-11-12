#include <stdio.h>
#include "rend.h"

const char *rpass_name[NUM_RPASSES] = {
	"shadow",
	"geometry",
	"light",
	"blend"
};

struct renderer *init_rend_debug(void);
struct renderer *init_rend_level(void);

int rend_init(void)
{
	if(!(renderer[REND_DBG] = init_rend_debug())) {
		fprintf(stderr, "failed to create debug renderer\n");
		return -1;
	}
	if(!(renderer[REND_LEVEL] = init_rend_level())) {
		fprintf(stderr, "failed to create level renderer\n");
		return -1;
	}

	return 0;
}

void rend_destroy(void)
{
	int i;

	for(i=0; i<NUM_REND; i++) {
		if(renderer[i]->destroy) renderer[i]->destroy();
	}
}

void rend_reshape(int x, int y)
{
	int i;

	for(i=0; i<NUM_REND; i++) {
		if(renderer[i]->reshape) renderer[i]->reshape(x, y);
	}
}

void rend_begin(int rid, int pass)
{
	if(renderer[rid]->beginpass) renderer[rid]->beginpass(pass);
}

void rend_end(int rid, int pass)
{
	if(renderer[rid]->endpass) renderer[rid]->endpass(pass);
}

void rend_pass(int rid, int pass, struct scene *scn)
{
	if(renderer[rid]->rendpass[pass]) {
		renderer[rid]->rendpass[pass](scn);
	}
}
