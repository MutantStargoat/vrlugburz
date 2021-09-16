#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <treestore.h>
#include "level.h"
#include "fs.h"

static int load_tileset(struct level *lvl, struct ts_node *tsn);

int init_level(struct level *lvl, int xsz, int ysz)
{
	memset(lvl, 0, sizeof *lvl);

	if(!(lvl->cells = calloc(xsz * ysz, sizeof *lvl->cells))) {
		free(lvl);
		return -1;
	}
	lvl->width = xsz;
	lvl->height = ysz;
	lvl->cell_size = DEF_CELL_SIZE;
	return 0;
}

void destroy_level(struct level *lvl)
{
	if(!lvl) return;
	free(lvl->cells);
	free(lvl->fname);
	free(lvl->dirname);
}

int load_level(struct level *lvl, const char *fname)
{
	struct ts_node *ts, *node, *iter;
	int i, j, sz, cx, cy;
	struct cell *cell;
	float *vecptr;

	lvl->fname = strdup(fname);
	if((lvl->dirname = malloc(strlen(fname) + 1))) {
#ifndef LEVEL_EDITOR
		path_dir(lvl->fname, lvl->dirname);
#endif
	}

	if(!(ts = ts_load(fname))) {
		fprintf(stderr, "failed to load level: %s\n", fname);
		return -1;
	}
	if(strcmp(ts->name, "dunged_level") != 0) {
		fprintf(stderr, "invalid or corrupted level file: %s\n", fname);
		ts_free_tree(ts);
		return -1;
	}

	if((sz = ts_get_attr_int(ts, "size", 0)) <= 0) {
		sz = 32;
	}

	if(init_level(lvl, sz, sz) == -1) {
		fprintf(stderr, "failed to initialize a %dx%d level\n", sz, sz);
		ts_free_tree(ts);
		return -1;
	}
	lvl->cell_size = ts_get_attr_num(ts, "cellsize", DEF_CELL_SIZE);

	if((vecptr = ts_get_attr_vec(ts, "player", 0))) {
		lvl->px = vecptr[0];
		lvl->py = vecptr[1];
	}

	iter = ts->child_list;
	while(iter) {
		node = iter;
		iter = iter->next;

		if(strcmp(node->name, "tileset") == 0) {
			load_tileset(lvl, node);

		} else if(strcmp(node->name, "cell") == 0) {
			cx = ts_get_attr_int(node, "x", -1);
			cy = ts_get_attr_int(node, "y", -1);
			if(cx < 0 || cy < 0 || cx >= sz || cy >= sz) {
				fprintf(stderr, "ignoring cell with invalid or missing coordinates\n");
				continue;
			}
			cell = lvl->cells + cy * sz + cx;
			cell->type = ts_get_attr_int(node, "blocked", 0) ? CELL_BLOCKED : CELL_WALK;

			/* abuse the next pointer to hang the treestore node temporarilly */
			cell->next = (struct cell*)node;
		}
	}

	/* assign wall types to all occupied cells */
	cell = lvl->cells;
	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			if(cell->type == CELL_SOLID) {
				cell++;
				continue;
			}

			/* TODO take wall choice from the level file into account */
			/* TODO detect corners */
			node = (struct ts_node*)cell->next;
			cell->next = 0;

			if(j <= 0 || cell[-1].type == CELL_SOLID) {
				cell->wall[0] = TILE_STRAIGHT;
			}
			if(i <= 0 || cell[-lvl->width].type == CELL_SOLID) {
				cell->wall[1] = TILE_STRAIGHT;
			}
			if(j >= lvl->width - 1 || cell[1].type == CELL_SOLID) {
				cell->wall[2] = TILE_STRAIGHT;
			}
			if(i >= lvl->height - 1 || cell[lvl->width].type == CELL_SOLID) {
				cell->wall[3] = TILE_STRAIGHT;
			}

			cell++;
		}
	}

	ts_free_tree(ts);
	return 0;
}

/* TODO: save tileset info */
int save_level(struct level *lvl, const char *fname)
{
	int i, j;
	struct ts_node *root, *node;
	struct ts_attr *attr;
	struct cell *cell;

	if(!(root = ts_alloc_node()) || ts_set_node_name(root, "dunged_level") == -1) {
		goto err;
	}

	if(!(attr = ts_alloc_attr()) || ts_set_attr_name(attr, "size") == -1) {
		ts_free_attr(attr);
		goto err;
	}
	ts_set_valuei(&attr->val, lvl->width);
	ts_add_attr(root, attr);

	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			cell = lvl->cells + i * lvl->width + j;
			if(cell->type == CELL_SOLID) continue;

			if(!(node = ts_alloc_node()) || ts_set_node_name(node, "cell") == -1) {
				ts_free_node(node);
				goto err;
			}
			if(!(attr = ts_alloc_attr()) || ts_set_attr_name(attr, "x") == -1) {
				ts_free_attr(attr);
				goto err;
			}
			ts_set_valuei(&attr->val, j);
			ts_add_attr(node, attr);
			if(!(attr = ts_alloc_attr()) || ts_set_attr_name(attr, "y") == -1) {
				ts_free_attr(attr);
				goto err;
			}
			ts_set_valuei(&attr->val, i);
			ts_add_attr(node, attr);

			if(cell->type == CELL_BLOCKED) {
				if(!(attr = ts_alloc_attr()) || ts_set_attr_name(attr, "blocked") == -1) {
					ts_free_attr(attr);
					goto err;
				}
				ts_set_valuei(&attr->val, 1);
				ts_add_attr(node, attr);
			}

			ts_add_child(root, node);
		}
	}

	if(ts_save(root, fname) == -1) {
		fprintf(stderr, "failed to save level: %s\n", fname);
		ts_free_tree(root);
		return -1;
	}
	ts_free_tree(root);
	return 0;

err:
	fprintf(stderr, "failed to construct treestore tree\n");
	ts_free_tree(root);
	return -1;
}

#ifndef LEVEL_EDITOR

static int load_tileset(struct level *lvl, struct ts_node *tsn)
{
	static const char *tile_types[] = {"empty", "straight", "corner", "door", 0};

	int i;
	char *path;
	const char *str;
	struct ts_node *node;
	struct tile *tile;

	node = tsn->child_list;
	while(node) {
		if(strcmp(node->name, "tile") == 0) {
			if(!(tile = calloc(1, sizeof *tile))) {
				fprintf(stderr, "failed to allocate tile\n");
				return -1;
			}
			if((str = ts_get_attr_str(node, "name", 0))) {
				tile->name = strdup(str);
			}
			if((str = ts_get_attr_str(node, "type", 0))) {
				for(i=0; tile_types[i]; i++) {
					if(strcmp(str, tile_types[i]) == 0) {
						tile->type = i;
						break;
					}
				}
			}
			if((str = ts_get_attr_str(node, "scene", 0))) {
				if(lvl->dirname) {
					path = alloca(strlen(lvl->dirname) + strlen(str) + 2);
					combine_path(lvl->dirname, str, path);
				} else {
					path = (char*)str;
				}
				load_scenefile(&tile->scn, path);
			}

			if(tile->name && tile->scn.meshlist) {	/* valid tile */
				tile->next = lvl->tiles;
				lvl->tiles = tile;
			} else {
				fprintf(stderr, "load_tileset: skipping invalid tile: %s\n",
						tile->name ? tile->name : "missing tile name");
				free(tile);
			}
		}
		node = node->next;
	}

	return 0;
}

struct tile *find_level_tile(struct level *lvl, int type)
{
	struct tile *tile = lvl->tiles;
	while(tile) {
		if(tile->type == type) {
			return tile;
		}
		tile = tile->next;
	}
	return 0;
}

int gen_cell_geom(struct level *lvl, struct cell *cell)
{
	int i;
	struct meshgroup *wallgeom;
	struct tile *tstr;
	struct mesh *mesh, *tmesh;
	float xform[16];

	if(!(tstr = find_level_tile(lvl, TILE_STRAIGHT))) {
		return -1;
	}

	if(!(wallgeom = malloc(sizeof *wallgeom))) {
		return -1;
	}
	init_meshgroup(wallgeom);

	for(i=0; i<4; i++) {
		if(cell->wall[i] == TILE_STRAIGHT) {	/* TODO: support other wall types */
			cgm_mrotation_y(xform, i * M_PI / 2.0f);

			tmesh = tstr->scn.meshlist;
			while(tmesh) {
				if(!(mesh = malloc(sizeof *mesh))) {
					return -1;
				}

				/* create a copy of the tile mesh */
				if(copy_mesh(mesh, tmesh) == -1) {
					free(mesh);
					return -1;
				}
				if(i) xform_mesh(mesh, xform);	/* rotate it to match the wall angle */

				/* add it to the level meshlist */
				mesh->next = lvl->meshlist;
				lvl->meshlist = mesh;

				/* add it to the meshgroup */
				if(add_meshgroup_mesh(wallgeom, mesh) == -1) {
					destroy_mesh(mesh);
					free(mesh);
					return -1;
				}

				tmesh = tmesh->next;
			}
		}
	}

	/* TODO: append to other existing meshgroups for detail objects */
	cell->mgrp = wallgeom;
	cell->num_mgrp = 1;

	return 0;
}

int gen_level_geom(struct level *lvl)
{
	int i, j;
	struct cell *cell;

	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			cell = lvl->cells + i * lvl->width + j;
			if(cell->type != CELL_SOLID) {
				if(gen_cell_geom(lvl, cell) == -1) {
					printf("failed to generate cell\n");
					return -1;
				}
			}
		}
	}
	return 0;
}

#else

static int load_tileset(struct level *lvl, struct ts_node *tsn)
{
	return 0;		/* in the level editor we don't need tileset loading */
}

#endif	/* !LEVEL_EDITOR */
