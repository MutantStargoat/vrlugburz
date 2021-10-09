#ifndef PROP_H_
#define PROP_H_

#include "scene.h"

enum {
	PROP_GRAB,
	PROP_SIM
};

struct prop {
	char *name;
	unsigned int flags;
	struct scene scn;

	struct prop *next;
};

int load_props(const char *fname);		/* can be called multiple times */
void free_props(void);

struct prop *find_prop(const char *name);	/* find a prop by name */

struct prop *dup_prop(const char *name);	/* allocate own copy of a prop */
void free_prop(struct prop *p);

#endif	/* PROP_H_ */
