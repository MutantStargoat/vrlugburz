#ifndef SCENEFILE_H_
#define SCENEFILE_H_

#include "mesh.h"

struct scenefile {
	char *fname;

	struct mesh *meshlist;
	int num_meshes;

	struct material *mtllist;
	int num_mtl;
};

int load_scenefile(struct scenefile *scn, const char *fname);
void destroy_scenefile(struct scenefile *scn);

struct mesh *find_mesh_prefix(struct scenefile *scn, const char *prefix);

#endif	/* SCENEFILE_H_ */
