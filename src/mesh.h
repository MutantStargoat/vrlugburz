#ifndef MESH_H_
#define MESH_H_

#include "cgmath/cgmath.h"
#include "geom.h"
#include "texture.h"

enum {
	MESH_ATTR_VERTEX,
	MESH_ATTR_NORMAL,
	MESH_ATTR_TANGENT,
	MESH_ATTR_TEXCOORD
};

struct vertex {
	cgm_vec3 pos;
	cgm_vec3 norm;
	cgm_vec3 tang;
	cgm_vec2 tex;
};

enum {
	TEX_DIFFUSE,
	TEX_SPECULAR,
	TEX_NORMAL,

	NUM_TEX_SLOTS
};

struct material {
	char *name;

	cgm_vec3 color;
	cgm_vec3 spec;
	float shininess;

	struct texture *tex[NUM_TEX_SLOTS];

	struct material *next;
};

struct mesh {
	char *name;

	float xform[16];

	struct vertex *varr;
	unsigned int *iarr;
	int num_verts, num_idx;
	int max_verts, max_idx;

	struct material *mtl;

	struct aabox bb;
	int bbvalid;

	unsigned int vbo, ibo;
	int vbovalid;

	struct mesh *next;
};

struct meshgroup {
	/* doesn't own the meshes */
	struct mesh **meshes;
	int num_meshes, max_meshes;

	struct aabox bb;
	int bbvalid;

	int num_verts, num_idx;
	unsigned int vbo, ibo;
	int vbovalid;
};

void init_mesh(struct mesh *m);
void destroy_mesh(struct mesh *m);
void clear_mesh(struct mesh *m);
int copy_mesh(struct mesh *dest, struct mesh *src);

void init_meshgroup(struct meshgroup *mg);
void destroy_meshgroup(struct meshgroup *mg);
void clear_meshgroup(struct meshgroup *mg);

void calc_mesh_bounds(struct mesh *m);

int add_mesh_vertex(struct mesh *m, struct vertex *v);
int add_mesh_index(struct mesh *m, int idx);
int add_mesh_face(struct mesh *m, int va, int vb, int vc);

int add_meshgroup_mesh(struct meshgroup *mg, struct mesh *m);

void draw_mesh(struct mesh *m);
void draw_meshgroup(struct meshgroup *mg);

void xform_mesh(struct mesh *mesh, float *mat);

#endif	/* MESH_H_ */
