#ifndef TILESET_H_
#define TILESET_H_

#include "scenefile.h"

#define DEF_TILE_SIZE	3.0f

struct tile;

struct tileset {
	char *name, *fname;
	float tile_size;

	struct scenefile scn;	/* scene file containing tile geometry */
	struct tile *tiles;

	struct tileset *next;
};

int load_tileset(struct tileset *tset, const char *fname);
void destroy_tileset(struct tileset *tset);

struct tileset *get_tileset(const char *fname);
void free_all_tilesets(void);

struct tile *get_tile(struct tileset *tset, int ttype);

int tile_type(const char *tstr);

#endif	/* TILESET_H_ */
