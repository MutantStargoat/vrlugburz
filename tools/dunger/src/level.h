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


int init_level(struct level *lvl, int xsz, int ysz);
void destroy_level(struct level *lvl);

int load_level(struct level *lvl, const char *fname);
int save_level(struct level *lvl, const char *fname);

#endif	/* LEVEL_H_ */
