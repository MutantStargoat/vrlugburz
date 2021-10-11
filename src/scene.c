#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "util.h"
#include "darray.h"

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

void add_snode_mesh(struct snode *sn, struct mesh *m)
{
	darr_push(sn->meshes, &m);
}

void add_snode_light(struct snode *sn, struct light *lt)
{
	darr_push(sn->meshes, &lt);
}

void add_snode_scene(struct snode *sn, struct scene *scn)
{
	darr_push(sn->scenes, &scn);
}


int init_scene(struct scene *scn)
{
	memset(scn, 0, sizeof *scn);

	scn->root = malloc_nf(sizeof *scn->root);
	scn->meshes = darr_alloc(0, sizeof *scn->meshes);
	scn->lights = darr_alloc(0, sizeof *scn->lights);
	scn->mtl = darr_alloc(0, sizeof *scn->mtl);

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

	for(i=0; i<darr_size(scn->mtl); i++) {
		free(scn->mtl[i]->name);
		free(scn->mtl[i]);
	}
	darr_free(scn->mtl);

	free_snode_tree(scn->root);
}

void add_scene_mesh(struct scene *scn, struct mesh *m)
{
	darr_push(scn->meshes, &m);
}

void add_scene_light(struct scene *scn, struct light *lt)
{
	darr_push(scn->lights, &lt);
}

void add_scene_material(struct scene *scn, struct material *mtl)
{
	darr_push(scn->mtl, &mtl);
}

struct material *find_scene_material(struct scene *scn, const char *name)
{
	int i, num = darr_size(scn->mtl);

	for(i=0; i<num; i++) {
		if(strcmp(scn->mtl[i]->name, name) == 0) {
			return scn->mtl[i];
		}
	}
	return 0;
}
