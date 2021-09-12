#ifndef LEVEL_H_
#define LEVEL_H_

#include "scenefile.h"

enum {
	TILE_STRAIGHT,
	TILE_CORNER,
	TILE_DOOR
};

enum {
	CELL_SOLID,
	CELL_WALK,
	CELL_BLOCKED
};

struct tile {
	char *name;
	int type;
	struct scenefile scn;
	struct tile *next;
};

struct cell {
	int type;
	int wall[4];
	int floor, ceil;

	struct meshgroup mgrp;
};

struct level {
	char *fname, *dirname;

	int width, height;
	struct cell *cells;

	struct tile *tiles;
};


int init_level(struct level *lvl, int xsz, int ysz);
void destroy_level(struct level *lvl);

int load_level(struct level *lvl, const char *fname);
int save_level(struct level *lvl, const char *fname);

int gen_level_geom(struct level *lvl);

#endif	/* LEVEL_H_ */
