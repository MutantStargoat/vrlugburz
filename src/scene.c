#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "util.h"

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
	sn->meshlist = 0;
	sn->lights = 0;
	return 0;
}

void destroy_snode(struct snode *sn)
{
	anm_destroy_node(&sn->anm);
}

void add_snode_mesh(struct snode *sn, struct mesh *m)
{
	m->next = sn->meshlist;
	sn->meshlist = m;
}

void add_snode_light(struct snode *sn, struct light *lt)
{
	lt->next = sn->lights;
	sn->lights = lt;
}


int init_scene(struct scene *scn)
{
	memset(scn, 0, sizeof *scn);

	if(!(scn->root = malloc(sizeof *scn->root))) {
		return -1;
	}
	init_snode(scn->root);
	return 0;
}

void destroy_scene(struct scene *scn)
{
	int i;

	for(i=0; i<scn->num_meshes; i++) {
		destroy_mesh(scn->meshes[i]);
		free(scn->meshes[i]);
	}
	free(scn->meshes);

	for(i=0; i<scn->num_lights; i++) {
		free(scn->lights[i]);
	}
	free(scn->lights);

	for(i=0; i<scn->num_mtl; i++) {
		free(scn->mtl[i]);
	}
	free(scn->mtl);

	anm_free_node_tree((struct anm_node*)scn->root);
}

int add_scene_mesh(struct scene *scn, struct mesh *m)
{
	if(scn->num_meshes >= scn->max_meshes) {
		GROW_ARRAY(scn->meshes, scn->max_meshes, return -1);
	}
	scn->meshes[scn->num_meshes++] = m;
	return 0;
}

int add_scene_light(struct scene *scn, struct light *lt)
{
	if(scn->num_lights >= scn->max_lights) {
		GROW_ARRAY(scn->lights, scn->max_lights, return -1);
	}
	scn->lights[scn->num_lights++] = lt;
	return 0;
}

int add_scene_material(struct scene *scn, struct material *mtl)
{
	if(scn->num_mtl >= scn->max_mtl) {
		GROW_ARRAY(scn->mtl, scn->max_mtl, return -1);
	}
	scn->mtl[scn->num_mtl++] = mtl;
	return 0;
}
