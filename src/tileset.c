#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "treestore.h"
#include "tileset.h"
#include "level.h"
#include "fs.h"

static int match_prefix(const char *str, const char *prefix);

static struct tileset *tset_list;

int load_tileset(struct tileset *tset, const char *fname)
{
	struct ts_node *ts, *node, *iter;
	const char *str, *prefix;
	char *path;
	struct mesh *mesh;
	struct tile *tile;
	int type;
	float xform[16], *vec;

	if(!(ts = ts_load(fname))) {
		fprintf(stderr, "failed to load tileset: %s\n", fname);
		return -1;
	}
	if(strcmp(ts->name, "tileset") != 0) {
		fprintf(stderr, "invalid or corrupted tileset file: %s\n", fname);
		ts_free_tree(ts);
		return -1;
	}

	if(!(str = ts_get_attr_str(ts, "file", 0))) {
		fprintf(stderr, "tileset %s is missing the file attribute\n", fname);
		ts_free_tree(ts);
		return -1;
	}
	path = alloca(strlen(fname) + strlen(str) + 2);
	path_dir(fname, path);
	combine_path(path, str, path);

	if(load_scenefile(&tset->scn, path) == -1) {
		fprintf(stderr, "tileset %s: failed to load scene file: %s\n", fname, path);
		ts_free_tree(ts);
		return -1;
	}

	tset->fname = strdup(fname);
	tset->name = strdup(ts_get_attr_str(ts, "name", fname));

	tset->tile_size = ts_get_attr_num(ts, "tilesize", DEF_TILE_SIZE);

	iter = ts->child_list;
	while(iter) {
		node = iter;
		iter = iter->next;
		if(strcmp(node->name, "tile") == 0) {
			if(!(prefix = ts_get_attr_str(node, "prefix", 0))) {
				continue;
			}

			if(!(str = ts_get_attr_str(node, "type", 0))) {
				fprintf(stderr, "load_tileset: missing tile type\n");
				continue;
			}
			if((type = tile_type(str)) == -1) {
				fprintf(stderr, "load_tileset: invalid tile type: %s\n", str);
				continue;
			}

			if(!(tile = malloc(sizeof *tile))) {
				fprintf(stderr, "load_tileset: failed to allocate tile\n");
				continue;
			}
			tile->type = type;

			cgm_midentity(xform);
			if((vec = ts_get_attr_vec(node, "pos", 0))) {
				cgm_mtranslation(xform, -vec[0], -vec[1], -vec[2]);
			}

			init_meshgroup(&tile->mgrp);

			mesh = tset->scn.meshlist;
			while(mesh) {
				if(mesh->name && match_prefix(mesh->name, prefix)) {
					if(vec) {
						xform_mesh(mesh, xform);
					}
					add_meshgroup_mesh(&tile->mgrp, mesh);
				}
				mesh = mesh->next;
			}

			tile->next = tset->tiles;
			tset->tiles = tile;
		}
	}

	return 0;
}

void destroy_tileset(struct tileset *tset)
{
	struct tile *tile;

	free(tset->name);
	free(tset->fname);

	while(tset->tiles) {
		tile = tset->tiles;
		tset->tiles = tile->next;

		free(tile->name);
		free(tile);
	}

	destroy_scenefile(&tset->scn);
}

struct tileset *get_tileset(const char *fname)
{
	struct tileset *ts = tset_list;
	while(ts) {
		if(strcmp(ts->fname, fname) == 0) {
			return ts;
		}
		ts = ts->next;
	}

	if(!(ts = malloc(sizeof *ts))) {
		fprintf(stderr, "failed to allocate tileset\n");
		return 0;
	}
	if(load_tileset(ts, fname) == -1) {
		free(ts);
		return 0;
	}
	ts->next = tset_list;
	tset_list = ts;
	return ts;
}

void free_all_tilesets(void)
{
	struct tileset *ts;

	while(tset_list) {
		ts = tset_list;
		tset_list = ts->next;
		destroy_tileset(ts);
		free(ts);
	}
}

struct tile *get_tile(struct tileset *tset, int ttype)
{
	struct tile *tile = tset->tiles;
	while(tile) {
		if(tile->type == ttype) {
			return tile;
		}
		tile = tile->next;
	}
	return 0;
}

int tile_type(const char *tstr)
{
	static const char *typenames[] = {
		"open", "straight", "corner", "tee", "cross", "str2open", "stropen", 0
	};
	int i;

	if(!tstr) return -1;

	for(i=0; typenames[i]; i++) {
		if(strcmp(tstr, typenames[i]) == 0) {
			return i;
		}
	}
	return -1;
}

static int match_prefix(const char *str, const char *prefix)
{
	while(*str && *prefix) {
		if(*str++ != *prefix++) {
			return 0;
		}
	}
	return 1;
}
