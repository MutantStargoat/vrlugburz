#ifndef LEVEL_H_
#define LEVEL_H_

enum {
	CELL_SOLID,
	CELL_WALK,
	CELL_BLOCKED
};

struct cell {
	int type;
	int wall[4];
	int floor, ceil;
};

struct level {
	int width, height;
	struct cell *cells;
};


struct level *create_level(int xsz, int ysz);
void free_level(struct level *lvl);

void draw_level(struct level *lvl);

#endif	/* LEVEL_H_ */
