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

static unsigned int sdr;

struct renderer *init_rend_debug(void)
{
	if(!(sdr = create_program_load("sdr/foo.v.glsl", "sdr/foo.p.glsl"))) {
		return 0;
	}
	glBindAttribLocation(sdr, MESH_ATTR_VERTEX, "apos");
	glBindAttribLocation(sdr, MESH_ATTR_NORMAL, "anorm");
	glBindAttribLocation(sdr, MESH_ATTR_TANGENT, "atang");
	glBindAttribLocation(sdr, MESH_ATTR_TEXCOORD, "atex");
	link_program(sdr);

	return &rfunc;
}

static void destroy(void)
{
	free_program(sdr);
}

static void begin(int pass)
{
	if(pass == RPASS_GEOM) {
		glUseProgram(sdr);
	}
}

static void end(int pass)
{
	if(pass == RPASS_GEOM) {
		glUseProgram(0);
	}
}

static void draw(struct scene *scn)
{
	int i, num = darr_size(scn->meshes);

	for(i=0; i<num; i++) {
		draw_mesh(scn->meshes[i]);
	}
}
