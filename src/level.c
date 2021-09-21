#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <treestore.h>
#include "level.h"
#include "tileset.h"
#include "fs.h"

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
	lvl->px = lvl->py = -1;
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

	lvl->fname = strdup(fname);
	if((lvl->dirname = malloc(strlen(fname) + 1))) {
#ifndef LEVEL_EDITOR
		path_dir(lvl->fname, lvl->dirname);
#endif
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
			/* TODO */

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

			/*
			if(i >= lvl->height - 1 || cell[lvl->width].type == CELL_SOLID) {
				cell->wall[0] = TILE_STR;
			}
			if(j >= lvl->width - 1 || cell[1].type == CELL_SOLID) {
				cell->wall[1] = TILE_STR;
			}
			if(i <= 0 || cell[-lvl->width].type == CELL_SOLID) {
				cell->wall[2] = TILE_STR;
			}
			if(j <= 0 || cell[-1].type == CELL_SOLID) {
				cell->wall[3] = TILE_STR;
			}
			*/

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

	if(lvl->cell_size && (attr = ts_alloc_attr())) {
		ts_set_attr_name(attr, "cellsize");
		ts_set_valuef(&attr->val, lvl->cell_size);
		ts_add_attr(root, attr);
	}

	if(lvl->px >= 0 && lvl->px < lvl->width && lvl->py >= 0 && lvl->py < lvl->height) {
		if((attr = ts_alloc_attr())) {
			ts_set_attr_name(attr, "player");
			ts_set_valueiv(&attr->val, 2, lvl->px, lvl->py);
			ts_add_attr(root, attr);
		}
	}

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

int gen_cell_geom(struct level *lvl, struct cell *cell)
{
#if 0
	int i;
	struct meshgroup *wallgeom;
	struct tile *tstr;
	struct mesh *mesh, *tmesh;
	float xform[16];

	if(!(tstr = find_level_tile(lvl, TILE_STR))) {
		return -1;
	}

	if(!(wallgeom = malloc(sizeof *wallgeom))) {
		return -1;
	}
	init_meshgroup(wallgeom);

	for(i=0; i<4; i++) {
		if(cell->wall[i] == TILE_STR) {	/* TODO: support other wall types */
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
#endif
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

#endif	/* !LEVEL_EDITOR */
