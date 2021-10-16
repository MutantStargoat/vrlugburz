#ifndef PROP_H_
#define PROP_H_

#include "scene.h"

enum {
	PROP_GRAB	= 1,
	PROP_SIM	= 2
};

struct prop {
	char *name;
	unsigned int flags;
	struct scene scn;

	struct prop *next;
};

void init_prop(struct prop *pr);
void destroy_prop(struct prop *pr);

struct prop *alloc_prop(void);
void free_prop(struct prop *p);

int load_props(const char *fname);		/* can be called multiple times */
void free_props(void);

struct prop *find_prop(const char *name);	/* find a prop by name */

struct prop *dup_prop(const char *name);	/* allocate own copy of a prop */

#endif	/* PROP_H_ */
