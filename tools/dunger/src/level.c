#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "level.h"

struct level *create_level(int xsz, int ysz)
{
	struct level *lvl;

	if(!(lvl = malloc(sizeof *lvl))) {
		return 0;
	}
	if(!(lvl->cells = calloc(xsz * ysz, sizeof *lvl->cells))) {
		free(lvl);
		return 0;
	}
	lvl->width = xsz;
	lvl->height = ysz;
	return lvl;
}

void free_level(struct level *lvl)
{
	if(!lvl) return;
	free(lvl->cells);
	free(lvl);
}
