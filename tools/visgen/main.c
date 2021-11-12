#include <stdio.h>
#include <stdlib.h>
#include "treestore.h"
#include "cgmath/cgmath.h"

struct cell {
	int x, y;
	struct cell *vislist;
	struct cell *next;
};

struct level {
	int width, height;
	int *cells;
	struct cell *clist;
};

void calc_visibility(struct level *lvl);
int calc_cell_visibility(struct level *lvl, struct cell *fromcell, int tx, int ty);
void add_vis_cell(struct cell *cell, int x, int y);
int is_solid(struct level *lvl, int x, int y);
void dump_cells(struct level *lvl, const char *fname);

#define zalloc(sz)	zalloc_impl(sz, __FILE__, __LINE__)
void *zalloc_impl(size_t sz, const char *file, int line);

int main(int argc, char **argv)
{
	int i, x, y;
	const char *inpath = 0, *outpath = 0;
	struct ts_node *ts, *node;
	struct ts_attr *attr, *aprev, dummy_attr;
	struct level lvl = {0};
	struct cell *cell, *vis;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 'o':
				if(!argv[++i]) {
					fprintf(stderr, "-o must be followed by the output file\n");
					return 1;
				}
				outpath = argv[i];
				break;

			case 'h':
				printf("Usage: %s [options] <level file>\n", argv[i]);
				printf("Options:\n");
				printf(" -o <path>: specify output file (default stdout)\n");
				printf(" -h: print usage and exit\n");
				return 0;

			default:
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return 1;
			}
		} else {
			if(inpath) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				return 1;
			}
			inpath = argv[i];
		}
	}

	if(!(ts = inpath ? ts_load(inpath) : ts_load_file(stdin))) {
		fprintf(stderr, "failed to read level %s\n", inpath ? inpath : "from stdin");
		return 1;
	}
	if(strcmp(ts->name, "dunged_level") != 0) {
		fprintf(stderr, "invalid or corrupted level file\n");
		return 1;
	}
	if((lvl.width = lvl.height = ts_get_attr_int(ts, "size", 0)) <= 0) {
		fprintf(stderr, "invalid or missing level size\n");
		return 1;
	}

	lvl.cells = zalloc(lvl.width * lvl.height * sizeof *lvl.cells);

	node = ts->child_list;
	while(node) {
		if(strcmp(node->name, "cell") == 0) {
			x = ts_get_attr_int(node, "x", -1);
			y = ts_get_attr_int(node, "y", -1);
			if(x < 0 || y < 0 || x >= lvl.width || y >= lvl.height) {
				fprintf(stderr, "ignoring invalid cell\n");
				node = node->next;
				continue;
			}
			if(lvl.cells[y * lvl.width + x]) {
				fprintf(stderr, "warning: duplicate cell %d,%d\n", x, y);
			} else {
				lvl.cells[y * lvl.width + x] = 1;
				cell = zalloc(sizeof *cell);
				cell->x = x;
				cell->y = y;
				cell->next = lvl.clist;
				lvl.clist = cell;
			}
		}
		node = node->next;
	}

	calc_visibility(&lvl);

	node = ts->child_list;
	while(node) {
		if(strcmp(node->name, "cell") != 0) {
			node = node->next;
			continue;
		}

		/* remove all existing vis attributes */
		dummy_attr.next = node->attr_list;
		aprev = &dummy_attr;
		while(aprev->next) {
			attr = aprev->next;
			if(strcmp(attr->name, "vis") == 0) {
				aprev->next = attr->next;
				if(!aprev->next) {
					node->attr_tail = aprev;
				}
				ts_free_attr(attr);
			} else {
				aprev = aprev->next;
			}
		}
		node->attr_list = dummy_attr.next;
		if(node->attr_tail == &dummy_attr) {
			node->attr_tail = 0;
		}

		x = ts_get_attr_int(node, "x", -1);
		y = ts_get_attr_int(node, "y", -1);
		if(x < 0 || y < 0 || x >= lvl.width || y >= lvl.height) {
			node = node->next;
			continue;
		}

		/* add visibility information */
		cell = lvl.clist;
		while(cell) {
			if(cell->x == x && cell->y == y) {
				vis = cell->vislist;
				while(vis) {
					if(!(attr = ts_alloc_attr()) || ts_set_attr_name(attr, "vis") == -1) {
						fprintf(stderr, "failed to allocate visibility attribute\n");
						abort();
					}
					ts_set_valuefv(&attr->val, 2, (float)vis->x, (float)vis->y);
					ts_add_attr(node, attr);

					vis = vis->next;
				}
				break;
			}
			cell = cell->next;
		}

		node = node->next;
	}

	if(outpath) {
		if(ts_save(ts, outpath) == -1) {
			fprintf(stderr, "failed to save file: %s\n", outpath);
			ts_free_tree(ts);
			return 1;
		}
	} else {
		ts_save_file(ts, stdout);
	}

	ts_free_tree(ts);
	return 0;
}

void calc_visibility(struct level *lvl)
{
	struct cell *from, *to;
	from = lvl->clist;
	while(from) {
		to = lvl->clist;
		while(to) {
			if(calc_cell_visibility(lvl, from, to->x, to->y)) {
				add_vis_cell(from, to->x, to->y);
			}
			to = to->next;
		}
		from = from->next;
	}
}

int check_vis_line(struct level *lvl, int x0, int y0, int x1, int y1)
{
	long x, y, dx, dy, slope, step;

	if(x0 == x1 && y0 == y1) return 1;

	x = x0 << 8;
	y = y0 << 8;
	dx = (x1 - x0) << 8;
	dy = (y1 - y0) << 8;

	if(abs(dx) > abs(dy)) {
		slope = dx ? (dy << 8) / dx : 0;
		if(dx >= 0) {
			step = 128;
		} else {
			step = -128;
			slope = -slope;
		}
		slope >>= 1;
		do {
			x += step;
			y += slope;
			if(is_solid(lvl, x >> 8, y >> 8)) return 0;
		} while(x >> 8 != x1);
	} else {
		slope = dy ? (dx << 8) / dy : 0;
		if(dy >= 0) {
			step = 128;
		} else {
			step = -128;
			slope = -slope;
		}
		slope >>= 1;
		do {
			x += slope;
			y += step;
			if(is_solid(lvl, x >> 8, y >> 8)) return 0;
		} while(y >> 8 != y1);
	}
	return 1;
}

int calc_cell_visibility(struct level *lvl, struct cell *fromcell, int tx, int ty)
{
	int i, j, ax, ay, bx, by;

	for(i=0; i<4; i++) {
		ax = fromcell->x + (i & 1);
		ay = fromcell->y + ((i >> 1) & 1);

		for(j=0; j<4; j++) {
			bx = tx + (j & 1);
			by = ty + ((j >> 1) & 1);

			if(check_vis_line(lvl, ax, ay, bx, by)) {
				return 1;
			}
		}
	}

	/* no unoccluded path was found */
	return 0;
}

void add_vis_cell(struct cell *cell, int x, int y)
{
	struct cell *vis = zalloc(sizeof *vis);
	vis->x = x;
	vis->y = y;
	vis->next = cell->vislist;
	cell->vislist = vis;
}

int is_solid(struct level *lvl, int x, int y)
{
	//printf("%d,%d %s\n", x, y, lvl->cells[y * lvl->width + x] ? "empty" : "solid");
	return lvl->cells[y * lvl->width + x] == 0;
}

void *zalloc_impl(size_t sz, const char *file, int line)
{
	void *p;
	if(!(p = calloc(1, sz))) {
		fprintf(stderr, "%s:%d failed to allocate %lu bytes\n", file, line, (unsigned long)sz);
		abort();
	}
	return p;
}

void dump_cells(struct level *lvl, const char *fname)
{
	int i, j;
	int *cptr;
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "dump_cells: failed to open: %s\n", fname);
		return;
	}
	fprintf(fp, "P5\n%d %d\n255\n", lvl->width, lvl->height);

	cptr = lvl->cells;
	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			fputc(*cptr++ ? 255 : 0, fp);
		}
	}

	fclose(fp);
}
