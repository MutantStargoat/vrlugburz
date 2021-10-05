#ifndef SCENE_H_
#define SCENE_H_

#include "anim/anim.h"
#include "cgmath/cgmath.h"
#include "mesh.h"

struct light {
	char *name;

	cgm_vec3 pos;
	cgm_vec3 color;
	struct {
		float range;
		float freq;
		float phase;
	} flicker;

	struct light *next;
};

struct snode {
	struct anm_node anm;	/* this must be first to treat snode and anm_node interchangably */

	struct mesh *meshlist;
	struct light *lights;
};

#define SNODE_PARENT(sn)	(struct snode*)((sn)->anm.parent)
#define SNODE_CHILD(sn)		(struct snode*)((sn)->anm.child)
#define SNODE_NEXT(sn)		(struct snode*)((sn)->anm.next)

struct scene {
	char *fname;

	struct mesh **meshes;
	int num_meshes, max_meshes;

	struct light **lights;
	int num_lights, max_lights;

	struct material **mtl;
	int num_mtl, max_mtl;

	struct snode *root;
};

void init_light(struct light *lt);

int init_snode(struct snode *sn);
void destroy_snode(struct snode *sn);

void add_snode_mesh(struct snode *sn, struct mesh *m);
void add_snode_light(struct snode *sn, struct light *lt);

int init_scene(struct scene *scn);
void destroy_scene(struct scene *scn);

int add_scene_mesh(struct scene *scn, struct mesh *m);
int add_scene_light(struct scene *scn, struct light *lt);
int add_scene_material(struct scene *scn, struct material *mtl);

int load_scenefile(struct scene *scn, const char *fname);


#endif /* SCENE_H_ */
