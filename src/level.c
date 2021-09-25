#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <treestore.h>
#include "level.h"
#include "tileset.h"
#include "fs.h"

static int detect_cell_tile(struct level *lvl, int x, int y, int *rot);


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
	int i, j, sz, cx, cy, tiletype;
	struct cell *cell;
	float *vecptr;
	const char *str;
	char *tset_path;

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
		path_dir(lvl->fname, lvl->dirname);
	}

	lvl->cell_size = ts_get_attr_num(ts, "cellsize", DEF_CELL_SIZE);

	if((vecptr = ts_get_attr_vec(ts, "player", 0))) {
		lvl->px = vecptr[0];
		lvl->py = vecptr[1];
	}

	if((str = ts_get_attr_str(ts, "tileset", 0))) {
		tset_path = alloca(strlen(str) + strlen(lvl->dirname) + 2);
		combine_path(lvl->dirname, str, tset_path);
		lvl->tset = get_tileset(tset_path);
	}

	iter = ts->child_list;
	while(iter) {
		node = iter;
		iter = iter->next;

		if(strcmp(node->name, "cell") == 0) {
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

	cell = lvl->cells;
	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			if(cell->type == CELL_SOLID) {
				cell++;
				continue;
			}

			node = (struct ts_node*)cell->next;
			cell->next = 0;

			if((tiletype = tile_type(ts_get_attr_str(node, "tiletype", 0))) == -1) {
				/* no tile-type specified, try to guess */
				tiletype = detect_cell_tile(lvl, j, i, &cell->tilerot);
			}

			if(lvl->tset) {
				cell->tile = get_tile(lvl->tset, tiletype);
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

static int get_cell_type(struct level *lvl, int x, int y)
{
	if(x < 0 || x >= lvl->width || y < 0 || y >= lvl->height) {
		return CELL_SOLID;
	}
	return lvl->cells[y * lvl->width + x].type;
}

static int detect_cell_tile(struct level *lvl, int x, int y, int *rot)
{
	int i, j, bit;
	unsigned int adj = 0;

	bit = 0;
	for(i=0; i<3; i++) {
		for(j=0; j<3; j++) {
			if(i == 1 && j == 1) continue;
			if(get_cell_type(lvl, x + j - 1, y + i - 1) == CELL_SOLID) {
				adj |= 1 << bit;
			}
			bit++;
		}
	}

	*rot = 0;

	switch(adj) {
	case 0:
	case 0757:
		/* really we'd need a separate tile type for "all neighbors solid", but we
		 * probably never going to need that in practice, so fuck it.
		 */
		return TILE_OPEN;

	case 0555:	/* N-S corridor */
		*rot = 1;
	case 0707:	/* W-E corridor */
		return TILE_STR;

	case 0745:	/* S-E corner */
		*rot = 1;
	case 0715:	/* S-W corner */
		return TILE_CORNER;
	case 0547:	/* N-E corner */
		*rot = 2;
		return TILE_CORNER;
	case 0517:	/* N-W corner */
		*rot = 3;
		return TILE_CORNER;

	case 0507:	/* N tee */
		*rot = 3;
	case 0515:	/* W tee */
		return TILE_TEE;
	case 0705:	/* S tee */
		*rot = 1;
		return TILE_TEE;
	case 0545:	/* E tee */
		*rot = 2;
		return TILE_TEE;

	case 0505:	/* cross */
		return TILE_CROSS;

	case 0700:	/* S stropen */
	case 0701:
	case 0704:
		return TILE_STROPEN;
	case 0444:	/* E stropen */
	case 0445:
	case 0544:
		*rot = 1;
		return TILE_STROPEN;
	case 0007:	/* N stropen */
	case 0407:
	case 0107:
		*rot = 2;
		return TILE_STROPEN;
	case 0111:	/* W stropen */
	case 0511:
	case 0115:
		*rot = 3;
		return TILE_STROPEN;

	case 0404:	/* E str2open */
		return TILE_STR2OPEN;
	case 0005:	/* N str2open */
		*rot = 1;
		return TILE_STR2OPEN;
	case 0101:	/* W str2open */
		*rot = 2;
		return TILE_STR2OPEN;
	case 0500:	/* S str2open */
		*rot = 3;
		return TILE_STR2OPEN;
	}

	return TILE_OPEN;
}

#endif	/* !LEVEL_EDITOR */
