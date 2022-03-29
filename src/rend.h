#ifndef REND_H_
#define REND_H_

struct scene;

enum {
	REND_DBG,
	REND_LEVEL,
	REND_PRE,

	NUM_REND
};

enum {
	RPASS_SHADOW,
	RPASS_GEOM,
	RPASS_LIGHT,
	RPASS_BLEND,

	NUM_RPASSES
};

struct renderer {
	void (*destroy)(void);

	void (*reshape)(int, int);

	void (*beginpass)(int);
	void (*endpass)(int);

	void (*rendpass[NUM_RPASSES])(struct scene*);
};

struct renderer *renderer[NUM_REND];
extern const char *rpass_name[NUM_RPASSES];

int rend_init(void);
void rend_destroy(void);
void rend_reshape(int x, int y);

void rend_begin(int rid, int pass);
void rend_end(int rid, int pass);
void rend_pass(int rid, int pass, struct scene *scn);

#endif	/* REND_H_ */
