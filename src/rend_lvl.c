#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"
#include "game.h"
#include "rend.h"
#include "scene.h"
#include "rtarg.h"
#include "sdr.h"

static void destroy(void);
static void reshape(int x, int y);
static void begin(int pass);
static void end(int pass);
static void shadow_pass(struct scene *scn);
static void geom_pass(struct scene *scn);
static void light_pass(struct scene *scn);
static void blend_pass(struct scene *scn);

static struct renderer rfunc = {
	destroy,
	reshape,
	begin, end,
	{ shadow_pass, geom_pass, light_pass, blend_pass }
};

static struct render_target rtarg;
static unsigned int sdr_geom, sdr_light;

struct renderer *init_rend_level(void)
{
	if(!(sdr_geom = create_program_load("sdr/defer_geom.v.glsl", "sdr/defer_geom.p.glsl"))) {
		return 0;
	}
	glBindAttribLocation(sdr_geom, MESH_ATTR_VERTEX, "apos");
	glBindAttribLocation(sdr_geom, MESH_ATTR_NORMAL, "anorm");
	glBindAttribLocation(sdr_geom, MESH_ATTR_TANGENT, "atang");
	glBindAttribLocation(sdr_geom, MESH_ATTR_TEXCOORD, "atex");
	link_program(sdr_geom);

	if(!(sdr_light = create_program_load("sdr/defer_light.v.glsl", "sdr/defer_light.p.glsl"))) {
		free_program(sdr_geom);
		return 0;
	}

	if(init_rtarg(&rtarg, win_width, win_height, 4) == -1) {
		free_program(sdr_geom);
		free_program(sdr_light);
		return 0;
	}

	return &rfunc;
}

static void destroy(void)
{
	destroy_rtarg(&rtarg);
	free_program(sdr_geom);
	free_program(sdr_light);
}

static void reshape(int x, int y)
{
	resize_rtarg(&rtarg, x, y);
}

static void begin(int pass)
{
	switch(pass) {
	case RPASS_GEOM:
		bind_rtarg(&rtarg);
		glUseProgram(sdr_geom);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;

	case RPASS_LIGHT:
		glUseProgram(sdr_light);
		break;

	default:
		break;
	}
}

static void end(int pass)
{
	switch(pass) {
	case RPASS_GEOM:
		bind_rtarg(0);
		glUseProgram(0);
		break;

	case RPASS_LIGHT:
		glUseProgram(0);
		break;

	default:
		break;
	}
}

static void shadow_pass(struct scene *scn)
{
}

static void geom_pass(struct scene *scn)
{
	int i, num = darr_size(scn->meshes);

	for(i=0; i<num; i++) {
		glPushMatrix();
		glMultMatrixf(scn->meshes[i]->xform);
		draw_mesh(scn->meshes[i]);
		glPopMatrix();
	}
}

static void light_pass(struct scene *scn)
{
}

static void blend_pass(struct scene *scn)
{
}

void rend_lvl_debugvis(void)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef((float)rtarg.width / (float)rtarg.tex_width, (float)rtarg.height / (float)rtarg.tex_height, 1);

	glPushAttrib(GL_ENABLE_BIT);

	glBindTexture(GL_TEXTURE_2D, rtarg.tex[1]);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glUseProgram(0);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(-1, 1);
	glEnd();

	glPopAttrib();

	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
