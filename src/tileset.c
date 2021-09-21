#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "treestore.h"
#include "tileset.h"
#include "level.h"

int load_tileset(struct tileset *tset, const char *fname)
{
	struct ts_node *ts, *node;
	const char *str;
	char *path;

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
	path_dir(str, path);
	combine_path(path, str, path);

	if(load_scenefile(&tset->scn, path) == -1) {
		fprintf(stderr, "tileset %s: failed to load scene file: %s\n", fname, path);
		ts_free_tree(ts);
		return -1;
	}

	tset->name = strdup(ts_get_attr_str(ts, "name", fname));
}
