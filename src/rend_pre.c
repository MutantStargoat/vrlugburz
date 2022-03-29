#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"
#include "rend.h"
#include "scene.h"
#include "sdr.h"

static void destroy(void);
static void begin(int pass);
static void end(int pass);
static void draw(struct scene *scn);

static struct renderer rfunc = {
	destroy,
	0,
	begin, end,
	{ 0, draw, 0, 0 }
};

enum {
	SDR_USE_TEX	= 1,

	NUM_SDR_VARIANTS
};

static unsigned int sdr[NUM_SDR_VARIANTS];

struct renderer *init_rend_pre(void)
{
	int i;

	for(i=0; i<NUM_SDR_VARIANTS; i++) {
		clear_shader_header(GL_FRAGMENT_SHADER);

		if(i & SDR_USE_TEX) {
			add_shader_header(GL_FRAGMENT_SHADER, "#define USE_TEX");
		}
		if(!(sdr[i] = create_program_load("sdr/pre.v.glsl", "sdr/pre.p.glsl"))) {
			return 0;
		}
		glBindAttribLocation(sdr[i], MESH_ATTR_VERTEX, "apos");
		glBindAttribLocation(sdr[i], MESH_ATTR_NORMAL, "anorm");
		glBindAttribLocation(sdr[i], MESH_ATTR_TANGENT, "atang");
		glBindAttribLocation(sdr[i], MESH_ATTR_TEXCOORD, "atex");
		link_program(sdr[i]);
	}
	clear_shader_header(GL_FRAGMENT_SHADER);

	return &rfunc;
}

static void destroy(void)
{
	int i;
	for(i=0; i<NUM_SDR_VARIANTS; i++) {
		free_program(sdr[i]);
	}
}

static void begin(int pass)
{
}

static void end(int pass)
{
	if(pass == RPASS_GEOM) {
		glUseProgram(0);
	}
}

static void draw(struct scene *scn)
{
	int i, num;
	struct mesh *mesh;
	unsigned int sdrmask;

	num = darr_size(scn->meshes);
	for(i=0; i<num; i++) {
		if(!(mesh = scn->meshes[i])) continue;

		sdrmask = 0;

		if(mesh->mtl.tex[TEX_DIFFUSE]) {
			glBindTexture(GL_TEXTURE_2D, mesh->mtl.tex[TEX_DIFFUSE]->tex);
			sdrmask |= SDR_USE_TEX;
		}

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, &mesh->mtl.color.x);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &mesh->mtl.spec.x);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mesh->mtl.shininess);

		glUseProgram(sdr[sdrmask]);

		glPushMatrix();
		glMultMatrixf(scn->meshes[i]->xform);
		draw_mesh(scn->meshes[i]);
		glPopMatrix();
	}

	glActiveTexture(GL_TEXTURE0);
}
