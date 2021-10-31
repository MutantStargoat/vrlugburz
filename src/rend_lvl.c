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

static void create_light_sphere(struct mesh *mesh, float rad, int subdiv);
static void draw_light_sphere(struct light *lt);
static void draw_light_fullscr(void);


static struct renderer rfunc = {
	destroy,
	reshape,
	begin, end,
	{ shadow_pass, geom_pass, light_pass, blend_pass }
};

static struct render_target rtarg;
static unsigned int sdr_geom, sdr_light;
static int uloc_texscale, uloc_lpos, uloc_lcol;
static struct mesh sphmesh;

static unsigned int deftex[NUM_TEX_SLOTS];


struct renderer *init_rend_level(void)
{
	int i, j;
	uint32_t pixels[16];

	if(!(sdr_geom = create_program_load("sdr/defer_geom.v.glsl", "sdr/defer_geom.p.glsl"))) {
		return 0;
	}
	glBindAttribLocation(sdr_geom, MESH_ATTR_VERTEX, "apos");
	glBindAttribLocation(sdr_geom, MESH_ATTR_NORMAL, "anorm");
	glBindAttribLocation(sdr_geom, MESH_ATTR_TANGENT, "atang");
	glBindAttribLocation(sdr_geom, MESH_ATTR_TEXCOORD, "atex");
	link_program(sdr_geom);
	set_uniform_int(sdr_geom, "tex_color", TEX_DIFFUSE);
	set_uniform_int(sdr_geom, "tex_spec", TEX_SPECULAR);
	set_uniform_int(sdr_geom, "tex_norm", TEX_NORMAL);

	if(!(sdr_light = create_program_load("sdr/defer_light.v.glsl", "sdr/defer_light.p.glsl"))) {
		free_program(sdr_geom);
		return 0;
	}
	set_uniform_int(sdr_light, "gtex[0]", 0);
	set_uniform_int(sdr_light, "gtex[1]", 1);
	set_uniform_int(sdr_light, "gtex[2]", 2);
	set_uniform_int(sdr_light, "gtex[3]", 3);
	uloc_texscale = get_uniform_loc(sdr_light, "tex_scale");
	uloc_lpos = get_uniform_loc(sdr_light, "light_pos");
	uloc_lcol = get_uniform_loc(sdr_light, "light_color");

	if(init_rtarg(&rtarg, win_width, win_height, 4) == -1) {
		free_program(sdr_geom);
		free_program(sdr_light);
		return 0;
	}

	/* icosahedron circuimscribed over a unit sphere, is inscribed in a sphere
	 * with radius approximately 1.323169
	 */
	create_light_sphere(&sphmesh, 1.32317, 1);
	update_mesh_vbo(&sphmesh);

	/* generate default textures */
	glGenTextures(NUM_TEX_SLOTS, deftex);
	for(i=0; i<NUM_TEX_SLOTS; i++) {
		switch(i) {
		case TEX_DIFFUSE:
		case TEX_SPECULAR:
			memset(pixels, 0xff, sizeof pixels);
			break;

		case TEX_NORMAL:
			for(j=0; j<sizeof pixels / sizeof *pixels; j++) {
				pixels[j] = 0xffff7f7f;
			}
			break;
		}
		glBindTexture(GL_TEXTURE_2D, deftex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}

	return &rfunc;
}

static void destroy(void)
{
	destroy_rtarg(&rtarg);
	free_program(sdr_geom);
	free_program(sdr_light);
	destroy_mesh(&sphmesh);

	glDeleteTextures(NUM_TEX_SLOTS, deftex);
}

static void reshape(int x, int y)
{
	resize_rtarg(&rtarg, x, y);
}

static void begin(int pass)
{
	int i;

	switch(pass) {
	case RPASS_GEOM:
		bind_rtarg(&rtarg);
		glUseProgram(sdr_geom);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;

	case RPASS_LIGHT:
		/* TODO non-blit fallback */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, rtarg.fbo);
		glBlitFramebuffer(0, 0, win_width, win_height, 0, 0, win_width, win_height,
				GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		glUseProgram(sdr_light);
		if(uloc_texscale != -1) {
			float sx = (float)rtarg.width / ((float)win_width * (float)rtarg.tex_width);
			float sy = (float)rtarg.height / ((float)win_height * (float)rtarg.tex_height);
			glUniform2f(uloc_texscale, sx, sy);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		for(i=0; i<4; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, rtarg.tex[i]);
		}
		glActiveTexture(GL_TEXTURE0);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef((float)rtarg.width / rtarg.tex_width, (float)rtarg.height / rtarg.tex_height, 1);
		glMatrixMode(GL_MODELVIEW);
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
		glDisable(GL_BLEND);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
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
	int i, j, num = darr_size(scn->meshes);
	struct mesh *mesh;

	for(i=0; i<num; i++) {
		if(!(mesh = scn->meshes[i])) continue;

		for(j=0; j<NUM_TEX_SLOTS; j++) {
			glActiveTexture(GL_TEXTURE0 + j);
			if(mesh->mtl.tex[j]) {
				glBindTexture(GL_TEXTURE_2D, mesh->mtl.tex[j]->tex);
			} else {
				glBindTexture(GL_TEXTURE_2D, deftex[j]);
			}
		}

		glPushMatrix();
		glMultMatrixf(scn->meshes[i]->xform);
		draw_mesh(scn->meshes[i]);
		glPopMatrix();
	}

	glActiveTexture(GL_TEXTURE0);
}

static void light_pass(struct scene *scn)
{
	int i, num = darr_size(scn->lights);
	struct light *lt;
	cgm_vec3 lpos;
	float range;


	for(i=0; i<num; i++) {
		if((lt = scn->lights[i])) {
			lpos = scn->lights[i]->pos;
			cgm_vmul_m4v3(&lpos, world_matrix);
			cgm_vmul_m4v3(&lpos, view_matrix);
			if(uloc_lpos >= 0) {
				glUniform3f(uloc_lpos, lpos.x, lpos.y, lpos.z);
			}
			if(uloc_lcol >= 0) {
				glUniform3f(uloc_lcol, lt->color.x, lt->color.y, lt->color.z);
			}

			/* if we're inside the light's sphere, draw a fullscreen quad.
			 * we're inside if the view space light position is within the
			 * light's range plus the near clipping plane distance.
			 */
			range = lt->max_range + NEAR_CLIP;
			if(cgm_vlength_sq(&lpos) <= range * range) {
				draw_light_fullscr();
			} else {
				draw_light_sphere(scn->lights[i]);
			}
		}
	}
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


#define PHI		1.618034
static cgm_vec3 icosa_pt[] = {
	{PHI, 1, 0}, {-PHI, 1, 0}, {PHI, -1, 0}, {-PHI, -1, 0},
	{1, 0, PHI}, {1, 0, -PHI}, {-1, 0, PHI}, {-1, 0, -PHI},
	{0, PHI, 1}, {0, -PHI, 1}, {0, PHI, -1}, {0, -PHI, -1}
};
enum { P11, P12, P13, P14, P21, P22, P23, P24, P31, P32, P33, P34 };
static unsigned int icosa_idx[] = {
	P11, P31, P21, P11, P22, P33, P13, P21, P32, P13, P34, P22,
	P12, P23, P31, P12, P33, P24, P14, P32, P23, P14, P24, P34,
	P11, P33, P31, P12, P31, P33, P13, P32, P34, P14, P34, P32,
	P21, P13, P11, P22, P11, P13, P23, P12, P14, P24, P14, P12,
	P31, P23, P21, P32, P21, P23, P33, P22, P24, P34, P24, P22
};

static void geosphere(struct mesh *mesh, float rad, cgm_vec3 *v1, cgm_vec3 *v2,
		cgm_vec3 *v3, int iter)
{
	struct vertex v = {0};
	cgm_vec3 v12, v23, v31;

	if(!iter) {
		v.pos = v.norm = *v1;
		cgm_vscale(&v.pos, rad);
		add_mesh_vertex(mesh, &v);
		v.pos = v.norm = *v2;
		cgm_vscale(&v.pos, rad);
		add_mesh_vertex(mesh, &v);
		v.pos = v.norm = *v3;
		cgm_vscale(&v.pos, rad);
		add_mesh_vertex(mesh, &v);
		return;
	}

	cgm_vcons(&v12, v1->x + v2->x, v1->y + v2->y, v1->z + v2->z);
	cgm_vnormalize(&v12);
	cgm_vcons(&v23, v2->x + v3->x, v2->y + v3->y, v2->z + v3->z);
	cgm_vnormalize(&v23);
	cgm_vcons(&v31, v3->x + v1->x, v3->y + v1->y, v3->z + v1->z);
	cgm_vnormalize(&v31);

	iter--;
	geosphere(mesh, rad, v1, &v12, &v31, iter);
	geosphere(mesh, rad, v2, &v23, &v12, iter);
	geosphere(mesh, rad, v3, &v31, &v23, iter);
	geosphere(mesh, rad, &v12, &v23, &v31, iter);
}

static void create_light_sphere(struct mesh *mesh, float rad, int subdiv)
{
	int i, j, vidx;
	cgm_vec3 v[3];

	init_mesh(mesh);

	for(i=0; i<20; i++) {
		for(j=0; j<3; j++) {
			vidx = icosa_idx[i * 3 + j];
			v[j] = icosa_pt[vidx];
			cgm_vnormalize(v + j);
		}

		geosphere(mesh, rad, v, v + 1, v + 2, subdiv);
	}
}

static void draw_light_sphere(struct light *lt)
{
	glPushMatrix();
	glTranslatef(lt->pos.x, lt->pos.y, lt->pos.z);
	glScalef(lt->max_range, lt->max_range, lt->max_range);

	glBindBuffer(GL_ARRAY_BUFFER, sphmesh.vbo);
	glVertexPointer(3, GL_FLOAT, sizeof(struct vertex), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_TRIANGLES, 0, sphmesh.num_verts);

	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

static void draw_light_fullscr(void)
{
	glDisable(GL_DEPTH_TEST);

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}
