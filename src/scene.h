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

	float max_range;

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

	struct snode *root;
};

void init_light(struct light *lt);

int init_snode(struct snode *sn);
void destroy_snode(struct snode *sn);

struct snode *alloc_snode(void);
void free_snode(struct snode *sn);
void free_snode_tree(struct snode *sn);

void set_snode_name(struct snode *sn, const char *name);
const char *get_snode_name(struct snode *sn);

void add_snode_mesh(struct snode *sn, struct mesh *m);
void add_snode_light(struct snode *sn, struct light *lt);
void add_snode_scene(struct snode *sn, struct scene *scn);

int init_scene(struct scene *scn);
void destroy_scene(struct scene *scn);

void copy_scene(struct scene *dst, struct scene *src);

void add_scene_mesh(struct scene *scn, struct mesh *m);
void add_scene_light(struct scene *scn, struct light *lt);
/* just adds the node as a child of root */
void add_scene_node(struct scene *scn, struct snode *sn);

struct mesh *find_scene_mesh(struct scene *scn, const char *name);
struct mesh *find_scene_mesh_prefix(struct scene *scn, const char *prefix);
struct snode *find_scene_node(struct scene *scn, const char *name);

void upd_scene_xform(struct scene *scn, long tm);

int load_scenefile(struct scene *scn, const char *fname);


#endif /* SCENE_H_ */
