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
static unsigned int geom_sdr, light_sdr;

struct renderer *init_rend_level(void)
{
	if(init_rtarg(&rtarg, win_width, win_height, 4) == -1) {
		return 0;
	}
	return &rfunc;
}

static void destroy(void)
{
	destroy_rtarg(&rtarg);
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
		glUseProgram(geom_sdr);
		break;

	case RPASS_LIGHT:
		glUseProgram(light_sdr);
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
