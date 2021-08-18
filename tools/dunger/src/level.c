#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include "level.h"

extern int view_width, view_height;

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

static void draw_cell(float x, float y, float sz, struct cell *cell)
{
	static const float colors[][3] = {{0, 0, 0}, {0.6, 0.6, 0.6}, {0.4, 0.2, 0.1}};

	if(cell) {
		glColor3fv(colors[cell->type]);
	} else {
		glColor3f(1, 1, 1);
	}
	glVertex2f(x, y);
	glVertex2f(x + sz, y);
	glVertex2f(x + sz, y + sz);
	glVertex2f(x, y + sz);
}

void draw_level(struct level *lvl)
{
	int i, j;
	float x, y, dx, dy, cellsz;
	struct cell *cell;

	dx = view_width / lvl->width;
	dy = view_height / lvl->height;
	cellsz = dx > dy ? dy : dx;

	glBegin(GL_QUADS);
	cell = lvl->cells;
	y = 0;
	for(i=0; i<lvl->height; i++) {
		x = 0;
		for(j=0; j<lvl->width; j++) {
			draw_cell(x, y, cellsz, cell++);
			x += cellsz;
		}
		y += cellsz;
	}
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	y = 0;
	for(i=0; i<lvl->height; i++) {
		x = 0;
		for(j=0; j<lvl->width; j++) {
			draw_cell(x, y, cellsz, 0);
			x += cellsz;
		}
		y += cellsz;
	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
