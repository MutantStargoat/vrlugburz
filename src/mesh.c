#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#include "opengl.h"
#include "mesh.h"

static int update_mesh_vbo(struct mesh *m);
static int update_meshgroup_vbo(struct meshgroup *mg);

void init_mesh(struct mesh *m)
{
	memset(m, 0, sizeof *m);
}

void destroy_mesh(struct mesh *m)
{
	free(m->varr);
	free(m->iarr);

	if(m->vbo) {
		glDeleteBuffers(1, &m->vbo);
	}
	if(m->ibo) {
		glDeleteBuffers(1, &m->ibo);
	}
}

void clear_mesh(struct mesh *m)
{
	free(m->varr);
	free(m->iarr);

	m->varr = 0;
	m->iarr = 0;
	m->num_verts = m->max_verts = m->num_idx = m->max_idx = 0;
	m->mtl = 0;
	m->bbvalid = m->vbovalid = 0;
}

int copy_mesh(struct mesh *dest, struct mesh *src)
{
	init_mesh(dest);

	if(src->max_verts && !(dest->varr = malloc(src->max_verts * sizeof *dest->varr))) {
		return -1;
	}
	if(src->max_idx && !(dest->iarr = malloc(src->max_idx * sizeof *dest->iarr))) {
		free(dest->varr);
		dest->varr = 0;
		return -1;
	}

	dest->num_verts = src->num_verts;
	dest->max_verts = src->max_verts;
	if(dest->varr) {
		memcpy(dest->varr, src->varr, src->num_verts * sizeof *dest->varr);
	}

	dest->num_idx = src->num_idx;
	dest->max_idx = src->max_idx;
	if(dest->iarr) {
		memcpy(dest->iarr, src->iarr, src->num_idx * sizeof *dest->iarr);
	}

	return 0;
}

void init_meshgroup(struct meshgroup *mg)
{
	memset(mg, 0, sizeof *mg);
}

void destroy_meshgroup(struct meshgroup *mg)
{
	free(mg->meshes);

	if(mg->vbo) {
		glDeleteBuffers(1, &mg->vbo);
	}
	if(mg->ibo) {
		glDeleteBuffers(1, &mg->ibo);
	}
}

void clear_meshgroup(struct meshgroup *mg)
{
	free(mg->meshes);

	mg->meshes = 0;
	mg->num_meshes = 0;
	mg->num_verts = mg->num_idx = 0;

	mg->bbvalid = mg->vbovalid = 0;
}

void calc_mesh_bounds(struct mesh *m)
{
	int i;
	struct vertex *vptr = m->varr;

	m->bb.vmin.x = m->bb.vmin.y = m->bb.vmin.z = FLT_MAX;
	m->bb.vmax.x = m->bb.vmax.y = m->bb.vmax.z = -FLT_MAX;

	for(i=0; i<m->num_verts; i++) {
		if(vptr->pos.x < m->bb.vmin.x) m->bb.vmin.x = vptr->pos.x;
		if(vptr->pos.y < m->bb.vmin.y) m->bb.vmin.y = vptr->pos.y;
		if(vptr->pos.z < m->bb.vmin.z) m->bb.vmin.z = vptr->pos.z;
		if(vptr->pos.x > m->bb.vmax.x) m->bb.vmax.x = vptr->pos.x;
		if(vptr->pos.y > m->bb.vmax.y) m->bb.vmax.y = vptr->pos.y;
		if(vptr->pos.z > m->bb.vmax.z) m->bb.vmax.z = vptr->pos.z;
		vptr++;
	}

	m->bbvalid = 1;
}

void calc_meshgroup_bounds(struct meshgroup *mg)
{
	int i;
	struct mesh *m;

	mg->bb.vmin.x = mg->bb.vmin.y = mg->bb.vmin.z = FLT_MAX;
	mg->bb.vmax.x = mg->bb.vmax.y = mg->bb.vmax.z = -FLT_MAX;

	for(i=0; i<mg->num_meshes; i++) {
		m = mg->meshes[i];
		if(!m->bbvalid) {
			calc_mesh_bounds(m);
		}

		if(m->bb.vmin.x < mg->bb.vmin.x) mg->bb.vmin.x = m->bb.vmin.x;
		if(m->bb.vmin.y < mg->bb.vmin.y) mg->bb.vmin.y = m->bb.vmin.y;
		if(m->bb.vmin.z < mg->bb.vmin.z) mg->bb.vmin.z = m->bb.vmin.z;
		if(m->bb.vmax.x > mg->bb.vmax.x) mg->bb.vmax.x = m->bb.vmax.x;
		if(m->bb.vmax.y > mg->bb.vmax.y) mg->bb.vmax.y = m->bb.vmax.y;
		if(m->bb.vmax.z > mg->bb.vmax.z) mg->bb.vmax.z = m->bb.vmax.z;
	}

	mg->bbvalid = 1;
}

int add_mesh_vertex(struct mesh *m, struct vertex *v)
{
	void *tmp;
	int newmax;

	if(m->num_verts >= m->max_verts) {
		newmax = m->max_verts ? m->max_verts * 2 : 16;
		if(!(tmp = realloc(m->varr, newmax * sizeof *m->varr))) {
			return -1;
		}
		m->varr = tmp;
		m->max_verts = newmax;
	}

	m->varr[m->num_verts++] = *v;
	return 0;
}

int add_mesh_index(struct mesh *m, int idx)
{
	void *tmp;
	int newmax;

	if(m->num_idx >= m->max_idx) {
		newmax = m->max_idx ? m->max_idx * 2 : 16;
		if(!(tmp = realloc(m->iarr, newmax * sizeof *m->iarr))) {
			return -1;
		}
		m->iarr = tmp;
		m->max_idx = newmax;
	}

	m->iarr[m->num_idx++] = idx;
	return 0;
}

int add_mesh_face(struct mesh *m, int va, int vb, int vc)
{
	if(add_mesh_index(m, va) == -1) return -1;
	if(add_mesh_index(m, vb) == -1) {
		m->num_idx--;
		return -1;
	}
	if(add_mesh_index(m, vc) == -1) {
		m->num_idx -= 2;
		return -1;
	}
	return 0;
}

int add_meshgroup_mesh(struct meshgroup *mg, struct mesh *m)
{
	void *tmp;
	int newmax;

	if(mg->num_meshes >= mg->max_meshes) {
		newmax = mg->max_meshes ? mg->max_meshes * 2 : 16;
		if(!(tmp = realloc(mg->meshes, newmax * sizeof *mg->meshes))) {
			return -1;
		}
		mg->meshes = tmp;
		mg->max_meshes = newmax;
	}

	mg->meshes[mg->num_meshes++] = m;
	return 0;
}

void draw_mesh(struct mesh *m)
{
	if(!m->vbovalid) {
		if(update_mesh_vbo(m) == -1) {
			return;
		}
	}

	glEnableVertexAttribArray(MESH_ATTR_VERTEX);
	glEnableVertexAttribArray(MESH_ATTR_NORMAL);
	glEnableVertexAttribArray(MESH_ATTR_TANGENT);
	glEnableVertexAttribArray(MESH_ATTR_TEXCOORD);

	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glVertexAttribPointer(MESH_ATTR_VERTEX, 3, GL_FLOAT, 0, sizeof *m->varr, 0);
	glVertexAttribPointer(MESH_ATTR_NORMAL, 3, GL_FLOAT, 0, sizeof *m->varr, (void*)offsetof(struct vertex, norm));
	glVertexAttribPointer(MESH_ATTR_TANGENT, 3, GL_FLOAT, 0, sizeof *m->varr, (void*)offsetof(struct vertex, tang));
	glVertexAttribPointer(MESH_ATTR_TEXCOORD, 2, GL_FLOAT, 0, sizeof *m->varr, (void*)offsetof(struct vertex, tex));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(m->ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
		glDrawElements(GL_TRIANGLES, m->num_idx, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, m->num_verts);
	}

	glDisableVertexAttribArray(MESH_ATTR_VERTEX);
	glDisableVertexAttribArray(MESH_ATTR_NORMAL);
	glDisableVertexAttribArray(MESH_ATTR_TANGENT);
	glDisableVertexAttribArray(MESH_ATTR_TEXCOORD);
}

void draw_meshgroup(struct meshgroup *mg)
{
	if(!mg->vbovalid) {
		if(update_meshgroup_vbo(mg) == -1) {
			return;
		}
	}

	glEnableVertexAttribArray(MESH_ATTR_VERTEX);
	glEnableVertexAttribArray(MESH_ATTR_NORMAL);
	glEnableVertexAttribArray(MESH_ATTR_TANGENT);
	glEnableVertexAttribArray(MESH_ATTR_TEXCOORD);

	glBindBuffer(GL_ARRAY_BUFFER, mg->vbo);
	glVertexAttribPointer(MESH_ATTR_VERTEX, 3, GL_FLOAT, 0, sizeof(struct vertex), 0);
	glVertexAttribPointer(MESH_ATTR_NORMAL, 3, GL_FLOAT, 0, sizeof(struct vertex), (void*)offsetof(struct vertex, norm));
	glVertexAttribPointer(MESH_ATTR_TANGENT, 3, GL_FLOAT, 0, sizeof(struct vertex), (void*)offsetof(struct vertex, tang));
	glVertexAttribPointer(MESH_ATTR_TEXCOORD, 2, GL_FLOAT, 0, sizeof(struct vertex), (void*)offsetof(struct vertex, tex));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(mg->ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mg->ibo);
		glDrawElements(GL_TRIANGLES, mg->num_idx, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, mg->num_verts);
	}

	glDisableVertexAttribArray(MESH_ATTR_VERTEX);
	glDisableVertexAttribArray(MESH_ATTR_NORMAL);
	glDisableVertexAttribArray(MESH_ATTR_TANGENT);
	glDisableVertexAttribArray(MESH_ATTR_TEXCOORD);
}

static int update_mesh_vbo(struct mesh *m)
{
	if(m->num_verts <= 0) return -1;

	if(!m->vbo) {
		glGenBuffers(1, &m->vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, m->num_verts * sizeof *m->varr, m->varr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(m->num_idx > 0) {
		if(!m->ibo) {
			glGenBuffers(1, &m->ibo);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->num_idx * sizeof *m->iarr,
				m->iarr, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	return 0;
}

static int update_meshgroup_vbo(struct meshgroup *mg)
{
	int i, j, idx0 = 0;
	struct vertex *varr, *vptr;
	unsigned int *iarr = 0, *iptr;
	struct mesh *m;

	mg->num_verts = mg->num_idx = 0;

	for(i=0; i<mg->num_meshes; i++) {
		mg->num_verts += mg->meshes[i]->num_verts;
		mg->num_idx += mg->meshes[i]->num_idx;
	}

	if(mg->num_verts <= 0) return -1;

	if(!(varr = malloc(mg->num_verts * sizeof *varr))) {
		fprintf(stderr, "update_meshgroup_vbo: failed to allocate vertex array with %d vertices\n", mg->num_verts);
		return -1;
	}
	if(mg->num_idx > 0) {
		if(!(iarr = malloc(mg->num_idx * sizeof *iarr))) {
			fprintf(stderr, "update_meshgroup_vbo: failed to allocate index array with %d indices\n", mg->num_idx);
			free(varr);
			return -1;
		}
	}

	vptr = varr;
	iptr = iarr;

	for(i=0; i<mg->num_meshes; i++) {
		m = mg->meshes[i];
		memcpy(vptr, m->varr, m->num_verts * sizeof *vptr);
		vptr += m->num_verts;

		if(iarr) {
			for(j=0; j<m->num_idx; j++) {
				*iptr++ = m->iarr[j] + idx0;
			}
			idx0 += m->num_idx;
		}
	}

	if(!mg->vbo) {
		glGenBuffers(1, &mg->vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mg->vbo);
	glBufferData(GL_ARRAY_BUFFER, mg->num_verts * sizeof *varr, varr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if(iarr) {
		if(!mg->ibo) {
			glGenBuffers(1, &mg->ibo);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mg->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mg->num_idx * sizeof *iarr, iarr, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	free(varr);
	free(iarr);
	return 0;
}

void xform_mesh(struct mesh *mesh, float *mat)
{
	int i;

	mesh->vbovalid = 0;
	mesh->bbvalid = 0;

	for(i=0; i<mesh->num_verts; i++) {
		cgm_vmul_v3m4(&mesh->varr[i].pos, mat);
		cgm_vmul_v3m3(&mesh->varr[i].norm, mat);
		cgm_vmul_v3m3(&mesh->varr[i].tang, mat);
	}
}
