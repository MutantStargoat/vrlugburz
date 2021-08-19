#ifndef LVIEW_H_
#define LVIEW_H_

#include "level.h"

int init_lview(struct level *lvl);
void destroy_lview(void);

void lview_viewport(int x, int y, int xsz, int ysz);

void pan_lview(float dx, float dy);
void zoom_lview(float dz);

void lview_mouse(int x, int y);

void draw_lview(void);

void cell_to_pos(int cx, int cy, float *px, float *py);
struct cell *pos_to_cell(float px, float py, int *cx, int *cy);

#endif	/* LVIEW_H_ */
