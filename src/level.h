#ifndef LEVEL_H_
#define LEVEL_H_

#include "scene.h"
#include "tileset.h"

#define DEF_CELL_SIZE	3.0f

enum {
	TILE_OPEN,
	TILE_STR,
	TILE_CORNER,
	TILE_OPENCORNER,
	TILE_TEE,
	TILE_CROSS,
	TILE_STR2OPEN,
	TILE_STROPEN
};

enum {
	CELL_SOLID,
	CELL_WALK,
	CELL_BLOCKED
};

struct tile {
	char *name;
	int type;

	struct scene scn;

	struct tile *next;
};

struct cell {
	int x, y;
	int type;
	int tiletype, tilerot;
	int wall[4];
	int floor, ceil;

	struct tile *tile;
	struct meshgroup *mgrp;
	int num_mgrp;

	struct cell *next;
};

struct level {
	char *fname, *dirname;

	int width, height;
	int px, py;				/* player start position */
	float cell_size;
	struct cell *cells;

	struct tileset *tset;

	/* everything owned by the level, excluding anything in the tilesets which
	 * are reusable across levels
	 */
	struct scene scn;

	int visdist;
};


int init_level(struct level *lvl, int xsz, int ysz);
void destroy_level(struct level *lvl);

int load_level(struct level *lvl, const char *fname);
int save_level(struct level *lvl, const char *fname);

struct tile *find_level_tile(struct level *lvl, int type);

int gen_cell_geom(struct level *lvl, struct cell *cell);
int gen_level_geom(struct level *lvl);

int get_cell_type(struct level *lvl, int x, int y);

#endif	/* LEVEL_H_ */
