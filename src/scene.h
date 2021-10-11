#ifndef SCENE_H_
#define SCENE_H_

#include "anim/anim.h"
#include "cgmath/cgmath.h"
#include "mesh.h"
#include "darray.h"

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

	/* dynamic arrays with pointers to things that can be associated with nodes.
	 * not owned by snode
	 */
	struct mesh **meshes;
	struct light **lights;
	struct scene **scenes;
};

#define SNODE_PARENT(sn)	(struct snode*)((sn)->anm.parent)
#define SNODE_CHILD(sn)		(struct snode*)((sn)->anm.child)
#define SNODE_NEXT(sn)		(struct snode*)((sn)->anm.next)

struct scene {
	char *fname;

	/* dynamic arrays */
	struct mesh **meshes;
	struct light **lights;
	struct material **mtl;

	struct snode *root;
};

void init_light(struct light *lt);

int init_snode(struct snode *sn);
void destroy_snode(struct snode *sn);

void add_snode_mesh(struct snode *sn, struct mesh *m);
void add_snode_light(struct snode *sn, struct light *lt);
void add_snode_scene(struct snode *sn, struct scene *scn);

int init_scene(struct scene *scn);
void destroy_scene(struct scene *scn);

void add_scene_mesh(struct scene *scn, struct mesh *m);
void add_scene_light(struct scene *scn, struct light *lt);
void add_scene_material(struct scene *scn, struct material *mtl);

struct material *find_scene_material(struct scene *scn, const char *name);

int load_scenefile(struct scene *scn, const char *fname);


#endif /* SCENE_H_ */
