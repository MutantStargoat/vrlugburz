#ifndef SCENEFILE_H_
#define SCENEFILE_H_

#include "mesh.h"

struct scenefile {
	struct mesh *meshlist;
	int num_meshes;

	struct material *mtllist;
	int num_mtl;
};

int load_scenefile(struct scenefile *scn, const char *fname);
void destroy_scenefile(struct scenefile *scn);

#endif	/* SCENEFILE_H_ */
