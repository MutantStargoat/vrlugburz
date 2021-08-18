#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include "level.h"

extern int view_width, view_height;
extern float view_panx, view_pany, view_zoom;


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

#define LTHICK	0.5f
static void draw_cell(struct level *lvl, struct cell *cell)
{
	int cidx, row, col;
	float x, y, xsz, ysz, sz;
	static const float colors[][3] = {{0, 0, 0}, {0.6, 0.6, 0.6}, {0.4, 0.2, 0.1}};

	xsz = view_zoom * view_width / lvl->width;
	ysz = view_zoom * view_height / lvl->height;
	sz = xsz > ysz ? ysz : xsz;

	cidx = cell - lvl->cells;
	row = cidx / lvl->width;
	col = cidx % lvl->width;

	x = col * sz - view_panx;
	y = row * sz - view_pany;

	glColor3f(1, 1, 1);
	glVertex2f(x, y);
	glVertex2f(x + sz, y);
	glVertex2f(x + sz, y + sz);
	glVertex2f(x, y + sz);

	x += LTHICK;
	y += LTHICK;
	sz -= LTHICK * 2.0f;

	glColor3fv(colors[cell->type]);
	glVertex2f(x, y);
	glVertex2f(x + sz, y);
	glVertex2f(x + sz, y + sz);
	glVertex2f(x, y + sz);
}

void draw_level(struct level *lvl)
{
	int i, j;
	struct cell *cell;

	glBegin(GL_QUADS);
	cell = lvl->cells;
	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			draw_cell(lvl, cell++);
		}
	}
	glEnd();
}
