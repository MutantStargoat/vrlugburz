#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include "level.h"

extern int view_width, view_height;
extern float view_panx, view_pany, view_zoom;

extern int mousex, mousey, splitx;

static int cellsz;
static struct cell *selcell;


struct level *create_level(int xsz, int ysz)
{
	struct level *lvl;

	if(!(lvl = malloc(sizeof *lvl))) {
		return 0;
	}
	if(!(lvl->cells = calloc(xsz * ysz, sizeof *lvl->cells))) {
		free(lvl);
		return 0;
	}
	lvl->width = xsz;
	lvl->height = ysz;
	return lvl;
}

void free_level(struct level *lvl)
{
	if(!lvl) return;
	free(lvl->cells);
	free(lvl);
}

void cell_to_pos(struct level *lvl, int cx, int cy, float *px, float *py)
{
	if(px) *px = (cx - lvl->width / 2.0f) * cellsz - view_panx + view_width / 2.0f;
	if(py) *py = (cy - lvl->height / 2.0f) * cellsz - view_pany + view_height / 2.0f;
}

struct cell *pos_to_cell(struct level *lvl, float px, float py, int *cx, int *cy)
{
	int col, row;

	col = (px + view_panx - view_width / 2.0f) / cellsz + lvl->width / 2.0f;
	row = (py + view_pany - view_height / 2.0f) / cellsz + lvl->height / 2.0f;

	if(cx) *cx = col;
	if(cy) *cy = row;

	if(col >= 0 && col < lvl->width && row >= 0 && row < lvl->height) {
		return lvl->cells + row * lvl->width + col;
	}
	return 0;
}

#define LTHICK	0.5f
static void draw_cell(struct level *lvl, struct cell *cell)
{
	int cidx, row, col;
	float x, y, hsz;
	static const float colors[][3] = {{0, 0, 0}, {0.6, 0.6, 0.6}, {0.4, 0.2, 0.1}};

	hsz = cellsz * 0.5f;

	cidx = cell - lvl->cells;
	row = cidx / lvl->width;
	col = cidx % lvl->width;

	cell_to_pos(lvl, col, row, &x, &y);
	/*printf("c->p: %d,%d -> %f,%f\n", col, row, x, y);
	pos_to_cell(lvl, x, y, &col, &row);
	printf("p->c: %f,%f -> %d,%d\n", x, y, col, row);*/

	if(selcell == cell) {
		glColor3f(0.4, 1.0f, 0.4);
	} else {
		glColor3f(0.5f, 0.5f, 0.5f);
	}
	glVertex2f(x - hsz, y - hsz);
	glVertex2f(x + hsz, y - hsz);
	glVertex2f(x + hsz, y + hsz);
	glVertex2f(x - hsz, y + hsz);

	x += LTHICK / 2.0f;
	y += LTHICK / 2.0f;
	hsz -= LTHICK * 2.0f;

	glColor3fv(colors[cell->type]);
	glVertex2f(x - hsz, y - hsz);
	glVertex2f(x + hsz, y - hsz);
	glVertex2f(x + hsz, y + hsz);
	glVertex2f(x - hsz, y + hsz);
}

void draw_level(struct level *lvl)
{
	int i, j;
	float xsz, ysz, hsz;
	struct cell *cell;

	xsz = view_zoom * view_width / lvl->width;
	ysz = view_zoom * view_height / lvl->height;
	cellsz = xsz > ysz ? ysz : xsz;
	hsz = cellsz / 2.0f;

	selcell = pos_to_cell(lvl, mousex + hsz - splitx, view_height - mousey + hsz, 0, 0);

	glBegin(GL_QUADS);
	cell = lvl->cells;
	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			draw_cell(lvl, cell++);
		}
	}
	glEnd();
}
