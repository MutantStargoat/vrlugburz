#include <GL/gl.h>
#include <drawtext.h>
#include "lview.h"
#include "app.h"

static struct level *lvl;
static struct cell *sel;
static int vpx, vpy, vpw, vph;
static float panx, pany, zoom;
static float cellsz;	/* derived from zoom and level properties */

int init_lview(struct level *l)
{
	lvl = l;
	sel = 0;
	panx = 0;
	pany = -12;
	zoom = 1;
	zoom_lview(0);
	return 0;
}

void destroy_lview(void)
{
}

void lview_viewport(int x, int y, int xsz, int ysz)
{
	vpx = x;
	vpy = y;
	vpw = xsz;
	vph = ysz;
	zoom_lview(0);	/* recalc cell size */
}

void pan_lview(float dx, float dy)
{
	panx += dx;
	pany += dy;
}

void zoom_lview(float dz)
{
	float xsz, ysz;

	zoom += dz;
	xsz = zoom * vpw / lvl->width;
	ysz = zoom * vph / lvl->height;
	cellsz = xsz > ysz ? ysz : xsz;
}

static int bnstate[8];

void lview_mbutton(int bn, int press, int x, int y)
{
	int cx, cy;
	float hsz = cellsz / 2.0f;
	sel = pos_to_cell(x + hsz - vpx, vph - y + hsz - vpy, 0, 0);
	bnstate[bn] = press;

	if(press) {
		if(!sel) return;

		switch(tool) {
		case TOOL_DRAW:
			if(bn == 0) {
				sel->type = CELL_WALK;
			} else if(bn == 2) {
				sel->type = CELL_SOLID;
			}
			break;

		case TOOL_PSTART:
			cell_coords(sel, &cx, &cy);
			if(bn == 0) {
				if(sel->type == CELL_WALK) {
					lvl->px = cx;
					lvl->py = cy;
				}
			} else if(bn == 2) {
				if(lvl->px == cx && lvl->py == cy) {
					lvl->px = lvl->py = -1;
				}
			}
			break;
		}
	}
}

void lview_mouse(int x, int y)
{
	float hsz = cellsz / 2.0f;
	if(!(sel = pos_to_cell(x + hsz - vpx, vph - y + hsz - vpy, 0, 0))) {
		return;
	}

	switch(tool) {
	case TOOL_DRAW:
		if(bnstate[0]) {
			sel->type = CELL_WALK;
		} else if(bnstate[2]) {
			sel->type = CELL_SOLID;
		}
		break;
	}
}

#define LTHICK	0.5f
static void draw_cell(struct cell *cell)
{
	int row, col;
	float x, y, hsz;
	static const float colors[][3] = {{0, 0, 0}, {0.6, 0.6, 0.6}, {0.4, 0.2, 0.1}};

	hsz = cellsz * 0.5f;

	cell_coords(cell, &col, &row);
	cell_to_pos(col, row, &x, &y);

	if(sel == cell) {
		glColor3f(1, 1, 1);
	} else {
		if(col == lvl->px && row == lvl->py) {
			glColor3f(0, 1, 0);
		} else {
			glColor3f(0.5f, 0.5f, 0.5f);
		}
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


void draw_lview(void)
{
	int i, j, row, col;
	struct cell *cell;

	glBegin(GL_QUADS);
	cell = lvl->cells;
	for(i=0; i<lvl->height; i++) {
		for(j=0; j<lvl->width; j++) {
			draw_cell(cell++);
		}
	}
	glEnd();

	if(sel) {
		cell_coords(sel, &col, &row);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(10, 10, 0);

		glColor3f(1, 1, 1);
		dtx_printf("(%d, %d)", col, row);
		dtx_flush();

		glPopMatrix();
	}
}

void cell_to_pos(int cx, int cy, float *px, float *py)
{
	if(px) *px = (cx - lvl->width / 2.0f) * cellsz - panx + vpw / 2.0f;
	if(py) *py = (cy - lvl->height / 2.0f) * cellsz - pany + vph / 2.0f;
}

struct cell *pos_to_cell(float px, float py, int *cx, int *cy)
{
	int col, row;

	col = (px + panx - vpw / 2.0f) / cellsz + lvl->width / 2.0f;
	row = (py + pany - vph / 2.0f) / cellsz + lvl->height / 2.0f;

	if(cx) *cx = col;
	if(cy) *cy = row;

	if(col >= 0 && col < lvl->width && row >= 0 && row < lvl->height) {
		return lvl->cells + row * lvl->width + col;
	}
	return 0;
}

void cell_coords(struct cell *cell, int *col, int *row)
{
	int cidx = cell - lvl->cells;
	*row = cidx / lvl->width;
	*col = cidx % lvl->width;
}
