#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>
#include "util.h"
#include "cgmath/cgmath.h"
#include "scene.h"
#include "texture.h"
#include "rbtree.h"
#include "util.h"


struct facevertex {
	int vidx, tidx, nidx;
};

struct objmtl {
	char *name;
	cgm_vec3 ka, kd, ks, ke;
	float shin;
	float alpha;
	float ior;
	char *map_kd, *map_ke, *map_alpha;
	struct objmtl *next;
};

static int proc_facevert(struct mesh *mesh, struct facevertex *fv,
		cgm_vec3 *varr, cgm_vec3 *narr, cgm_vec2 *tarr, struct rbtree *rbtree);

static char *cleanline(char *s);
static char *parse_idx(char *ptr, int *idx, int arrsz);
static char *parse_face_vert(char *ptr, struct facevertex *fv, int numv, int numt, int numn);

static struct objmtl *load_mtllib(const char *path_prefix, const char *mtlfname);
static struct objmtl *find_material(struct objmtl *mlist, const char *name);
static void conv_mtl(struct material *mm, struct objmtl *om, const char *path_prefix);

static int cmp_facevert(const void *ap, const void *bp);
static void free_rbnode_key(struct rbnode *n, void *cls);


int load_scenefile(struct scene *scn, const char *fname)
{
	int i, nlines, res = -1;
	FILE *fp;
	char buf[256], *line, *ptr, *path_prefix;
	int varr_size, varr_max, narr_size, narr_max, tarr_size, tarr_max;
	cgm_vec3 v, *varr = 0, *narr = 0;
	cgm_vec2 *tarr = 0;
	struct facevertex fv[4];
	struct mesh *mesh = 0;
	char *sep;
	struct rbtree *rbtree = 0;
	struct objmtl *mtllist = 0, *mtl = 0;

	init_scene(scn);

	varr_size = varr_max = narr_size = narr_max = tarr_size = tarr_max = 0;
	varr = narr = 0;
	tarr = 0;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "load_scenefile: failed to open %s\n", fname);
		return -1;
	}

	if(!(rbtree = rb_create(cmp_facevert))) {
		fprintf(stderr, "load_scenefile: failed to create facevertex search tree\n");
		goto fail;
	}
	rb_set_delete_func(rbtree, free_rbnode_key, 0);

	strcpy(buf, fname);
	if((sep = strrchr(buf, '/'))) {
		sep[1] = 0;
	} else {
		buf[0] = 0;
	}
	path_prefix = alloca(strlen(buf) + 1);
	strcpy(path_prefix, buf);

	if(sep) {
		sep = (char*)fname + (sep - buf);
	}
	if(!(scn->fname = strdup(sep ? sep + 1 : fname))) {
		fprintf(stderr, "failed to allocate scenefile name buffer\n");
		goto fail;
	}

	if(!(mesh = malloc(sizeof *mesh))) {
		fprintf(stderr, "failed to allocate mesh\n");
		fclose(fp);
		return -1;
	}
	init_mesh(mesh);

	nlines = 0;
	while(fgets(buf, sizeof buf, fp)) {
		nlines++;
		if(!(line = cleanline(buf))) {
			continue;
		}

		switch(line[0]) {
		case 'v':
			v.x = v.y = v.z = 0.0f;
			if(sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z) < 2) {
				break;
			}
			if(isspace(line[1])) {
				if(varr_size >= varr_max) {
					GROW_ARRAY(varr, varr_max, goto fail);
				}
				varr[varr_size++] = v;
			} else if(line[1] == 't' && isspace(line[2])) {
				if(tarr_size >= tarr_max) {
					GROW_ARRAY(tarr, tarr_max, goto fail);
				}
				v.y = 1.0f - v.y;
				tarr[tarr_size++] = *(cgm_vec2*)&v;
			} else if(line[1] == 'n' && isspace(line[2])) {
				if(narr_size >= narr_max) {
					GROW_ARRAY(narr, narr_max, goto fail);
				}
				narr[narr_size++] = v;
			}
			break;

		case 'f':
			if(!isspace(line[1])) break;

			ptr = line + 2;

			for(i=0; i<3; i++) {
				if(!(ptr = parse_face_vert(ptr, fv + i, varr_size, tarr_size, narr_size))) {
					break;
				}
				if(proc_facevert(mesh, fv + i, varr, narr, tarr, rbtree) == -1) {
					break;
				}
			}

			if(parse_face_vert(ptr, fv + 3, varr_size, tarr_size, narr_size)) {
				proc_facevert(mesh, fv, varr, narr, tarr, rbtree);
				proc_facevert(mesh, fv + 2, varr, narr, tarr, rbtree);
				proc_facevert(mesh, fv + 3, varr, narr, tarr, rbtree);
			}
			break;

		case 'o':
		case 'g':
			if(mesh->num_verts) {
				if(mtl) conv_mtl(&mesh->mtl, mtl, path_prefix);
				add_scene_mesh(scn, mesh);

				if(!(mesh = malloc(sizeof *mesh))) {
					fprintf(stderr, "failed to allocate mesh\n");
					goto fail;
				}
				init_mesh(mesh);
			}
			if(mesh->name) free(mesh->name);
			mesh->name = strdup_nf(cleanline(line + 2));
			break;

		case 'm':
			if(memcmp(line, "mtllib", 6) == 0 && (line = cleanline(line + 6))) {
				mtllist = load_mtllib(path_prefix, line);
			}
			break;

		case 'u':
			if(memcmp(line, "usemtl", 6) == 0 && (line = cleanline(line + 6))) {
				mtl = find_material(mtllist, line);
			}
			break;

		default:
			break;
		}
	}

	if(mesh->num_verts) {
		if(mtl) conv_mtl(&mesh->mtl, mtl, path_prefix);
		add_scene_mesh(scn, mesh);
	} else {
		free(mesh);
	}
	mesh = 0;

	printf("load_scenefile %s: loaded %d meshes, %d vertices\n", scn->fname,
			darr_size(scn->meshes), varr_size);
	for(i=0; i<darr_size(scn->meshes); i++) {
		printf(" - %s\n", scn->meshes[i]->name ? scn->meshes[i]->name : "?");
	}

	res = 0;

	if(0) {
fail:
		free(scn->fname);
		scn->fname = 0;
	}

	while(mtllist) {
		mtl = mtllist;
		mtllist = mtllist->next;
		free(mtl->name);
		free(mtl);
	}

	fclose(fp);
	free(mesh);
	free(varr);
	free(narr);
	free(tarr);
	rb_free(rbtree);
	return res;
}

static int proc_facevert(struct mesh *mesh, struct facevertex *fv,
		cgm_vec3 *varr, cgm_vec3 *narr, cgm_vec2 *tarr, struct rbtree *rbtree)
{
	struct rbnode *node;
	unsigned int idx, newidx;
	struct facevertex *newfv;
	struct vertex v;

	if((node = rb_find(rbtree, &fv))) {
		idx = (unsigned int)(intptr_t)node->data;
		assert((int)idx < mesh->num_verts);

		printf("lala\n");
		add_mesh_index(mesh, idx);
		return 0;
	}

	newidx = mesh->num_verts;

	v.pos = varr[fv->vidx];
	if(fv->nidx >= 0) {
		v.norm = narr[fv->nidx];
	}
	if(fv->tidx >= 0) {
		v.tex = tarr[fv->tidx];
	}
	add_mesh_vertex(mesh, &v);
	add_mesh_index(mesh, newidx);

	if((newfv = malloc(sizeof *newfv))) {
		*newfv = *fv;
	}
	if(!newfv || rb_insert(rbtree, newfv, (void*)(intptr_t)newidx) == -1) {
		fprintf(stderr, "load_scenefile: failed to insert facevertex to rbtree\n");
		free(newfv);
		return -1;
	}
	return 0;
}

static char *cleanline(char *s)
{
	char *ptr;

	if((ptr = strchr(s, '#'))) *ptr = 0;

	while(*s && isspace(*s)) s++;
	ptr = s + strlen(s) - 1;
	while(ptr >= s && isspace(*ptr)) *ptr-- = 0;

	return *s ? s : 0;
}

static char *parse_idx(char *ptr, int *idx, int arrsz)
{
	char *endp;
	int val = strtol(ptr, &endp, 10);
	if(endp == ptr) return 0;

	if(val < 0) {	/* convert negative indices */
		*idx = arrsz + val;
	} else {
		*idx = val - 1;	/* indices in obj are 1-based */
	}
	return endp;
}

/* possible face-vertex definitions:
 * 1. vertex
 * 2. vertex/texcoord
 * 3. vertex//normal
 * 4. vertex/texcoord/normal
 */
static char *parse_face_vert(char *ptr, struct facevertex *fv, int numv, int numt, int numn)
{
	fv->tidx = fv->nidx = -1;

	if(!(ptr = parse_idx(ptr, &fv->vidx, numv)))
		return 0;
	if(*ptr != '/') return (!*ptr || isspace(*ptr)) ? ptr : 0;

	if(*++ptr == '/') {	/* no texcoord */
		++ptr;
	} else {
		if(!(ptr = parse_idx(ptr, &fv->tidx, numt)))
			return 0;
		if(*ptr != '/') return (!*ptr || isspace(*ptr)) ? ptr : 0;
		++ptr;
	}

	if(!(ptr = parse_idx(ptr, &fv->nidx, numn)))
		return 0;
	return (!*ptr || isspace(*ptr)) ? ptr : 0;
}

static struct objmtl *load_mtllib(const char *path_prefix, const char *mtlfname)
{
	FILE *fp;
	char buf[256], *line;
	struct objmtl *mlist = 0, *mtail = 0, *mtl = 0;

	if(path_prefix && *path_prefix) {
		sprintf(buf, "%s/%s", path_prefix, mtlfname);
	} else {
		strcpy(buf, mtlfname);
	}

	if(!(fp = fopen(buf, "rb"))) {
		return 0;
	}

	while(fgets(buf, sizeof buf, fp)) {
		if(!(line = cleanline(buf))) {
			continue;
		}

		if(memcmp(line, "newmtl", 6) == 0) {
			mtl = calloc_nf(1, sizeof *mtl);

			if((line = cleanline(line + 6))) {
				mtl->name = strdup_nf(line);
			}
			if(mlist) {
				mtail->next = mtl;
			} else {
				mlist = mtail = mtl;
			}
			printf("adding material: %s\n", mtl->name);

		} else if(memcmp(line, "Kd", 2) == 0) {
			sscanf(line + 3, "%f %f %f", &mtl->kd.x, &mtl->kd.y, &mtl->kd.z);
		} else if(memcmp(line, "Ks", 2) == 0) {
			sscanf(line + 3, "%f %f %f", &mtl->ks.x, &mtl->ks.y, &mtl->ks.z);
		} else if(memcmp(line, "Ke", 2) == 0) {
			sscanf(line + 3, "%f %f %f", &mtl->ke.x, &mtl->ke.y, &mtl->ke.z);
		} else if(memcmp(line, "Ni", 2) == 0) {
			mtl->ior = atof(line + 3);
		} else if(line[0] == 'd' && isspace(line[1])) {
			mtl->alpha = atof(line + 2);
		} else if(memcmp(line, "map_Kd", 6) == 0) {
			if((line = cleanline(line + 6))) {
				mtl->map_kd = strdup_nf(line);
			}
		} else if(memcmp(line, "map_Ke", 6) == 0) {
			if((line = cleanline(line + 6))) {
				mtl->map_ke = strdup_nf(line);
			}
		} else if(memcmp(line, "map_d", 5) == 0) {
			if((line = cleanline(line + 5))) {
				mtl->map_alpha = strdup_nf(line);
			}
		}
	}

	fclose(fp);
	return mlist;
}

static struct objmtl *find_material(struct objmtl *mlist, const char *name)
{
	while(mlist) {
		if(strcmp(mlist->name, name) == 0) {
			break;
		}
		mlist = mlist->next;
	}
	return mlist;
}

static void conv_mtl(struct material *mm, struct objmtl *om, const char *path_prefix)
{
	char *fname = 0, *suffix = 0;
	int len, prefix_len, maxlen = 0;

	memset(mm, 0, sizeof *mm);
	mm->name = strdup_nf(om->name);
	mm->color = om->kd;
	mm->spec = om->ks;
	mm->shininess = om->shin;

	if(om->map_kd && (len = strlen(om->map_kd)) > maxlen) maxlen = len;
	if(om->map_ke && (len = strlen(om->map_ke)) > maxlen) maxlen = len;
	if(om->map_alpha && (len = strlen(om->map_alpha)) > maxlen) maxlen = len;

	if(maxlen) {
		prefix_len = strlen(path_prefix);
		fname = alloca(maxlen + prefix_len + 2);
		suffix = fname + prefix_len;
		strcpy(fname, path_prefix);
	}

	if(om->map_kd) {
		strcpy(suffix, om->map_kd);
		mm->tex[TEX_DIFFUSE] = get_texture(fname);
	}
}

static int cmp_facevert(const void *ap, const void *bp)
{
	const struct facevertex *a = ap;
	const struct facevertex *b = bp;

	if(a->vidx == b->vidx) {
		if(a->tidx == b->tidx) {
			return a->nidx - b->nidx;
		}
		return a->tidx - b->tidx;
	}
	return a->vidx - b->vidx;
}

static void free_rbnode_key(struct rbnode *n, void *cls)
{
	free(n->key);
}
