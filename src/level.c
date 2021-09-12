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
	if(!(lvl->cells = calloc(xsz * ysz, sizeof *lvl->cells))) {
		free(lvl);
		return -1;
	}
	lvl->width = xsz;
	lvl->height = ysz;
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
	int sz, cx, cy;
	struct cell *cell;

	lvl->fname = strdup(fname);
	if((lvl->dirname = malloc(strlen(fname) + 1))) {
		path_dir(lvl->fname, lvl->dirname);
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
			/* TODO wall tiles and detail objects */
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

static int load_tileset(struct level *lvl, struct ts_node *tsn)
{
	static const char *tile_types[] = {"straight", "corner", "door", 0};

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
