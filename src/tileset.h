#ifndef TILESET_H_
#define TILESET_H_

#include "scenefile.h"

struct tile;

struct tileset {
	const char *name;

	struct scenefile scn;	/* scene file containing tile geometry */
	struct tile *tiles;
};

int load_tileset(struct tileset *tset, const char *fname);
void destroy_tileset(struct tileset *tset);

struct tileset *get_tileset(const char *name);

#endif	/* TILESET_H_ */
