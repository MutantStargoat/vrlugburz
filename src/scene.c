#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "util.h"
#include "darray.h"
#include "rbtree.h"

static struct snode *copy_snode_tree(struct snode *src, struct rbtree *objmap);

void init_light(struct light *lt)
{
	memset(lt, 0, sizeof *lt);
	cgm_vcons(&lt->color, 1, 1, 1);
}

int init_snode(struct snode *sn)
{
	if(anm_init_node(&sn->anm) == -1) {
		return -1;
	}

	sn->meshes = darr_alloc(0, sizeof *sn->meshes);
	sn->lights = darr_alloc(0, sizeof *sn->lights);
	sn->scenes = darr_alloc(0, sizeof *sn->scenes);
	return 0;
}

void destroy_snode(struct snode *sn)
{
	anm_destroy_node(&sn->anm);
	darr_free(sn->meshes);
	darr_free(sn->lights);
	darr_free(sn->scenes);
}

struct snode *alloc_snode(void)
{
	struct snode *sn = malloc_nf(sizeof *sn);
	init_snode(sn);
	return sn;
}

void free_snode(struct snode *sn)
{
	if(!sn) return;
	destroy_snode(sn);
	free(sn);
}

void free_snode_tree(struct snode *sn)
{
	struct anm_node *an;
	struct snode *cnode;

	if(!sn) return;

	an = sn->anm.child;
	while(an) {
		cnode = (struct snode*)an;
		an = an->next;
		free_snode_tree(cnode);
	}

	destroy_snode(sn);
	free(sn);
}

void set_snode_name(struct snode *sn, const char *name)
{
	anm_set_node_name(&sn->anm, name);
}

const char *get_snode_name(struct snode *sn)
{
	return anm_get_node_name(&sn->anm);
}

void add_snode_mesh(struct snode *sn, struct mesh *m)
{
	darr_push(sn->meshes, &m);
}

void add_snode_light(struct snode *sn, struct light *lt)
{
	darr_push(sn->lights, &lt);
}

void add_snode_scene(struct snode *sn, struct scene *scn)
{
	darr_push(sn->scenes, &scn);

	scn->root->anm.parent = &sn->anm;
}


int init_scene(struct scene *scn)
{
	memset(scn, 0, sizeof *scn);

	scn->root = malloc_nf(sizeof *scn->root);
	scn->meshes = darr_alloc(0, sizeof *scn->meshes);
	scn->lights = darr_alloc(0, sizeof *scn->lights);

	init_snode(scn->root);
	return 0;
}

void destroy_scene(struct scene *scn)
{
	int i;

	free(scn->fname);

	for(i=0; i<darr_size(scn->meshes); i++) {
		destroy_mesh(scn->meshes[i]);
		free(scn->meshes[i]);
	}
	darr_free(scn->meshes);

	for(i=0; i<darr_size(scn->lights); i++) {
		free(scn->lights[i]);
	}
	darr_free(scn->lights);

	free_snode_tree(scn->root);
}

void copy_scene(struct scene *dst, struct scene *src)
{
	int i;
	struct mesh *mesh;
	struct light *lt;
	struct rbtree *objmap;

	objmap = rb_create(RB_KEY_ADDR);

	dst->fname = src->fname ? strdup_nf(src->fname) : 0;

	for(i=0; i<darr_size(src->meshes); i++) {
		if(src->meshes[i]) {
			mesh = malloc_nf(sizeof *mesh);
			copy_mesh(mesh, src->meshes[i]);
			add_scene_mesh(dst, mesh);

			rb_insert(objmap, src->meshes[i], mesh);
		}
	}

	for(i=0; i<darr_size(src->lights); i++) {
		if(src->lights[i]) {
			lt = malloc_nf(sizeof *lt);
			init_light(lt);
			lt->name = src->lights[i]->name ? strdup_nf(src->lights[i]->name) : 0;
			lt->pos = src->lights[i]->pos;
			lt->color = src->lights[i]->color;
			lt->flicker = src->lights[i]->flicker;
			lt->max_range = src->lights[i]->max_range;
			add_scene_light(dst, lt);

			rb_insert(objmap, src->lights[i], lt);
		}
	}

	dst->root = copy_snode_tree(src->root, objmap);
	rb_free(objmap);
}

void add_scene_mesh(struct scene *scn, struct mesh *m)
{
	darr_push(scn->meshes, &m);
}

void add_scene_light(struct scene *scn, struct light *lt)
{
	darr_push(scn->lights, &lt);
}

void add_scene_node(struct scene *scn, struct snode *sn)
{
	anm_link_node(&scn->root->anm, &sn->anm);
}

struct mesh *find_scene_mesh(struct scene *scn, const char *name)
{
	int i, num = darr_size(scn->meshes);

	for(i=0; i<num; i++) {
		if(strcmp(scn->meshes[i]->name, name) == 0) {
			return scn->meshes[i];
		}
	}
	return 0;
}

struct mesh *find_scene_mesh_prefix(struct scene *scn, const char *prefix)
{
	int i, num = darr_size(scn->meshes);

	for(i=0; i<num; i++) {
		if(match_prefix(scn->meshes[i]->name, prefix)) {
			return scn->meshes[i];
		}
	}
	return 0;
}

static struct snode *find_node_rec(struct snode *n, const char *name)
{
	struct snode *cnode, *res;

	if(!n) return 0;

	if(n->anm.name && strcmp(n->anm.name, name) == 0) {
		return n;
	}

	cnode = SNODE_CHILD(n);
	while(cnode) {
		if((res = find_node_rec(cnode, name))) {
			return res;
		}
		cnode = SNODE_NEXT(cnode);
	}
	return 0;
}

struct snode *find_scene_node(struct scene *scn, const char *name)
{
	return find_node_rec(scn->root, name);
}

static void update_node(struct snode *node, long tm)
{
	int i, num;
	struct snode *cn;
	struct mesh *mesh;
	struct light *lt;

	anm_eval_node(&node->anm, tm);

	if(node->anm.parent) {
		cgm_mmul(node->anm.matrix, node->anm.parent->matrix);
	}

	/* update the mesh matrices */
	num = darr_size(node->meshes);
	for(i=0; i<num; i++) {
		mesh = node->meshes[i];
		if(mesh) {
			memcpy(mesh->xform, node->anm.matrix, sizeof mesh->xform);
		}
	}

	/* update light positions */
	num = darr_size(node->lights);
	for(i=0; i<num; i++) {
		lt = node->lights[i];
		if(lt) {
			lt->pos.x = lt->pos.y = lt->pos.z = 0.0f;
			cgm_vmul_m4v3(&lt->pos, node->anm.matrix);
		}
	}

	cn = SNODE_CHILD(node);
	while(cn) {
		update_node(cn, tm);
		cn = SNODE_NEXT(cn);
	}
}

void upd_scene_xform(struct scene *scn, long tm)
{
	update_node(scn->root, tm);
}


static struct snode *copy_snode_tree(struct snode *src, struct rbtree *objmap)
{
	int i, j, num;
	struct snode *node, *child;
	struct anm_animation *san, *dan;
	struct rbnode *rbnode;

	node = alloc_snode();

	/* objmap contains mappings between mesh/light/material pointers in the original
	 * node tree, and the node tree of the scene we're creating.  If no mapping
	 * is found for any given object, or no objmap is provided, then the
	 * pointers are copied to the new tree
	 */
	for(i=0; i<darr_size(src->meshes); i++) {
		if(!objmap || !(rbnode = rb_find(objmap, src->meshes[i]))) {
			add_snode_mesh(node, src->meshes[i]);
		} else {
			add_snode_mesh(node, rb_node_data(rbnode));
		}
	}
	for(i=0; i<darr_size(src->lights); i++) {
		if(!objmap || !(rbnode = rb_find(objmap, src->lights[i]))) {
			add_snode_light(node, src->lights[i]);
		} else {
			add_snode_light(node, rb_node_data(rbnode));
		}
	}
	for(i=0; i<darr_size(src->scenes); i++) {
		add_snode_scene(node, src->scenes[i]);
	}

	node->anm.name = src->anm.name ? strdup_nf(src->anm.name) : 0;
	node->anm.blend_dur = src->anm.blend_dur;
	node->anm.data = src->anm.data;

	num = anm_get_animation_count(&src->anm);
	for(i=0; i<num; i++) {
		san = anm_get_animation(&src->anm, i);
		anm_add_node_animation(&node->anm);
		dan = node->anm.animations + i;

		dan->name = san->name ? strdup_nf(san->name) : 0;
		for(j=0; j<ANM_NUM_TRACKS; j++) {
			anm_copy_track(dan->tracks + j, san->tracks + j);
		}
	}

	child = SNODE_CHILD(src);
	while(child) {
		anm_link_node(&node->anm, (struct anm_node*)copy_snode_tree(child, objmap));
		child = SNODE_NEXT(child);
	}
	return node;
}

