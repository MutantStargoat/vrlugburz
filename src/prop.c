#include <stdio.h>
#include <stdlib.h>
#include "prop.h"
#include "util.h"
#include "treestore.h"
#include "fs.h"

static int proc_prop_node(struct ts_node *pnode, struct scene *scn);

static struct prop *proplist;


void init_prop(struct prop *pr)
{
	memset(pr, 0, sizeof *pr);
	init_scene(&pr->scn);
}

void destroy_prop(struct prop *pr)
{
	free(pr->name);
	destroy_scene(&pr->scn);
}

struct prop *alloc_prop(void)
{
	struct prop *p = malloc_nf(sizeof *p);
	init_prop(p);
	return p;
}

void free_prop(struct prop *p)
{
	destroy_prop(p);
	free(p);
}


int load_props(const char *fname)
{
	int res = -1;
	struct ts_node *ts, *node;
	const char *str;
	char *datadir, *path;
	struct scene scn;

	datadir = alloca(strlen(fname) + 1);
	path_dir(fname, datadir);

	if(!(ts = ts_load(fname))) {
		fprintf(stderr, "failed to load props file: %s\n", fname);
		goto fail;
	}
	if(strcmp(ts->name, "props") != 0) {
		fprintf(stderr, "invalid props file: %s\n", fname);
		goto fail;
	}
	if(!(str = ts_get_attr_str(ts, "file", 0))) {
		fprintf(stderr, "missing \"file\" attribute from props file: %s\n", fname);
		goto fail;
	}

	path = alloca(strlen(datadir) + strlen(str) + 2);
	combine_path(datadir, str, path);
	if(load_scenefile(&scn, path) == -1) {
		fprintf(stderr, "failed to load scene file: %s\n", str);
		goto fail;
	}

	node = ts->child_list;
	while(node) {
		if(strcmp(node->name, "prop") == 0) {
			proc_prop_node(node, &scn);
		}
		node = node->next;
	}

	destroy_scene(&scn);
	res = 0;
fail:
	ts_free_tree(ts);
	return res;
}

static int proc_prop_node(struct ts_node *node, struct scene *scn)
{
	int i,num;
	struct mesh *mesh;
	struct light *lt;
	struct ts_attr *attr;
	struct ts_node *child;
	const char *prefix;
	struct snode *sn, *snpar;
	struct sphere sph;
	struct prop *prop;
	const float *vec;
	const char *str;

	if(!(prefix = ts_get_attr_str(node, "prefix", 0))) {
		fprintf(stderr, "ignoring prop without prefix\n");
		return -1;
	}

	prop = alloc_prop();
	prop->name = strdup_nf(ts_get_attr_str(node, "name", prefix));

	printf("loading prop: %s\n", prop->name);

	if(ts_get_attr_int(node, "grab", 0)) {
		prop->flags |= PROP_GRAB;
	}
	if(ts_get_attr_int(node, "sim", 0)) {
		prop->flags |= PROP_SIM;
	}

	/* first add empty nodes for every dummy */
	attr = node->attr_list;
	while(attr) {
		if(strcmp(attr->name, "dummy") == 0) {
			num = darr_size(scn->meshes);
			for(i=0; i<num; i++) {
				mesh = scn->meshes[i];
				if(!mesh || !mesh->name) continue;
				if(match_prefix(mesh->name, attr->val.str)) {
					/* found a dummy mesh. calculate bounds to position the node */
					calc_mesh_bounds(mesh);
					aabox_sphere_insc(&mesh->bb, &sph);

					printf(" - adding dummy %s: %f %f %f\n", mesh->name, sph.pos.x, sph.pos.y, sph.pos.z);

					/* destroy the mesh, no longer needed */
					destroy_mesh(mesh);
					scn->meshes[i] = 0;

					sn = alloc_snode();
					set_snode_name(sn, attr->val.str);
					anm_set_position(&sn->anm, &sph.pos.x, 0);
					add_scene_node(&prop->scn, sn);
				}
			}
		}
		attr = attr->next;
	}

	/* process child nodes */
	child = node->child_list;
	while(child) {
		if(strcmp(child->name, "light") == 0) {
			lt = malloc_nf(sizeof *lt);
			init_light(lt);

			sn = alloc_snode();
			add_snode_light(sn, lt);

			if((str = ts_get_attr_str(child, "parent", 0)) &&
					(snpar = find_scene_node(&prop->scn, str))) {
				anm_link_node(&snpar->anm, &sn->anm);
				printf(" - adding light under %s\n", str);
			} else {
				add_scene_node(&prop->scn, sn);
				printf(" - adding light\n");
			}

			if((vec = ts_get_attr_vec(child, "pos", 0))) {
				anm_set_position(&sn->anm, vec, 0);
			}

			if((vec = ts_get_attr_vec(child, "color", 0))) {
				cgm_vcons(&lt->color, vec[0], vec[1], vec[2]);
			}
		}

		child = child->next;
	}

	/* move over all the meshes matching the prefix */
	num = darr_size(scn->meshes);
	for(i=0; i<num; i++) {
		mesh = scn->meshes[i];
		if(!mesh || !mesh->name) continue;
		if(match_prefix(mesh->name, prefix)) {
			add_scene_mesh(&prop->scn, mesh);
			scn->meshes[i] = 0;
			printf(" - adding mesh: %s\n", mesh->name);
		}
	}

	if(darr_empty(prop->scn.meshes) && darr_empty(prop->scn.lights)) {
		fprintf(stderr, "ignoring empty prop: %s\n", prop->name);
		free_prop(prop);
		return -1;
	}

	prop->next = proplist;
	proplist = prop;
	return 0;
}

void free_props(void)
{
	struct prop *prop;

	while(proplist) {
		prop = proplist;
		proplist = proplist->next;
		free_prop(prop);
	}
}

struct prop *find_prop(const char *name)
{
	struct prop *prop;

	prop = proplist;
	while(prop) {
		if(strcmp(prop->name, name) == 0) {
			return prop;
		}
		prop = prop->next;
	}
	return 0;
}

struct prop *dup_prop(const char *name)
{
	struct prop *src, *prop;

	if(!(src = find_prop(name))) {
		return 0;
	}

	prop = alloc_prop();
	prop->name = strdup_nf(src->name);
	prop->flags = src->flags;

	copy_scene(&prop->scn, &src->scn);

	return prop;
}
