#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "level.h"
#include "player.h"
#include "rend.h"
#include "rtarg.h"
#include "sdr.h"
#include "debug.h"
#include "imago2.h"

#define REND	REND_PRE

static int init(void);
static void cleanup(void);
static int start(void);
static void stop(void);
static void update(float dt);
static void draw(void);
static void render_game(struct render_target *fbrt);
static void draw_level(int rpass);
static void resize(int x, int y);
static void keyboard(int key, int press);
static void mbutton(int bn, int press, int x, int y);
static void mmotion(int x, int y);

static struct screen scr = {
	"prerender",
	init, cleanup,
	start, stop,
	update,
	draw,
	resize,
	keyboard,
	mbutton,
	mmotion
};


struct level lvl;
struct player player;

int mouse_x, mouse_y;
int bnstate[8];

float cam_dist;

static long prev_step, prev_turn;

static struct render_target rtarg;
static unsigned int sdr_post;
static int uloc_expose;


void reg_pre_screen(void)
{
	assert(num_screens < MAX_SCREENS);
	screens[num_screens++] = &scr;
}


static int init(void)
{
	if((glcaps.caps & GLCAPS_FB_SRGB) && win_srgb) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	} else {
		add_shader_header(GL_FRAGMENT_SHADER, "#define FB_NOT_SRGB");
		printf("sRGB framebuffer is not supported, using post gamma fallback\n");
	}
	if(!(sdr_post = create_program_load("sdr/post.v.glsl", "sdr/post.p.glsl"))) {
		return -1;
	}
	uloc_expose = get_uniform_loc(sdr_post, "exposure");
	clear_shader_header(GL_FRAGMENT_SHADER);
	return 0;
}

static void cleanup(void)
{
	free_all_tilesets();
	free_props();
}

static int start(void)
{
	if(load_level(&lvl, "data/test.lvl") == -1) {
		return -1;
	}

	init_player(&player);
	player.lvl = &lvl;
	player.cx = -1;
	player.cy = 0;
	turn_player(&player, -1);
	player.phi = cgm_deg_to_rad(-9);

	vp_width = win_width;
	vp_height = win_height;

	if(init_rtarg(&rtarg, vp_width, vp_height, 1) == -1) {
		fprintf(stderr, "failed to create %dx%d framebuffer\n", vp_width, vp_height);
		return -1;
	}
	return 0;
}

static void stop(void)
{
	destroy_level(&lvl);
}

static int nextcell(void)
{
	int i, j;

	if(++player.cx >= lvl.width) {
		player.cx = 0;
		player.cy++;
	}

	for(i=player.cy; i<lvl.height; i++) {
		for(j=player.cx; j<lvl.width; j++) {
			if(get_cell_type(&lvl, j, i) == CELL_WALK) {
				player.cx = j;
				player.cy = i;
				return 0;
			}
		}
	}
	return -1;
}

static void update(float dt)
{
	turn_player(&player, 1);
	if(player.dir == 0) {
		player.dir = 0;
		if(nextcell() == -1) {
			exit(0);
		}
	}

	upd_player_xform(&player);
}

static void draw(void)
{
	char fname[64];
	struct img_pixmap img;

	cgm_midentity(proj_matrix);
	cgm_mperspective(proj_matrix, cgm_deg_to_rad(60), win_aspect, NEAR_CLIP, 500.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_matrix);

	cgm_midentity(view_matrix);
	cgm_mpretranslate(view_matrix, 0, 0, -cam_dist);
	cgm_mpremul(view_matrix, player.view_xform);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view_matrix);

	render_game(0);

	img_init(&img);
	if(img_set_pixels(&img, win_width, win_height, IMG_FMT_RGB24, 0) != -1) {
		sprintf(fname, "view_x%02dy%02dd%d.png", player.cx, player.cy, player.dir);
		glReadPixels(0, 0, win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, img.pixels);
		img_vflip(&img);
		img_save(&img, fname);
	}
	img_destroy(&img);

	assert(glGetError() == GL_NO_ERROR);
	game_swap_buffers();
}


static void render_game(struct render_target *fbrt)
{
	float sx, sy;

	dbg_begin();

	/* geometry pass */
	if(renderer[REND]->rendpass[RPASS_GEOM]) {
		if(!renderer[REND]->rendpass[RPASS_LIGHT]) {
			bind_rtarg(&rtarg);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		rend_begin(REND, RPASS_GEOM);
		draw_level(RPASS_GEOM);
		rend_end(REND, RPASS_GEOM);
	}

	/* light pass */
	if(renderer[REND]->rendpass[RPASS_LIGHT]) {
		bind_rtarg(&rtarg);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		rend_begin(REND, RPASS_LIGHT);
		draw_level(RPASS_LIGHT);
		rend_end(REND, RPASS_LIGHT);
	}

	/* post processing */
	bind_rtarg(fbrt);

	glDisable(GL_DEPTH_TEST);

	glUseProgram(sdr_post);
	if(uloc_expose >= 0) {
		glUniform1f(uloc_expose, 1.0f);
	}
	glBindTexture(GL_TEXTURE_2D, rtarg.tex[0]);

	sx = (float)rtarg.width / rtarg.tex_width;
	sy = (float)rtarg.height / rtarg.tex_height;

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(sx, 0);
	glVertex2f(1, -1);
	glTexCoord2f(sx, sy);
	glVertex2f(1, 1);
	glTexCoord2f(0, sy);
	glVertex2f(-1, 1);
	glEnd();

	glUseProgram(0);
	dbg_end();

	glEnable(GL_DEPTH_TEST);
}


static void draw_level(int rpass)
{
	int i, num_vis, count = 0;
	struct cell *cell, *pcell;
	static int last_rpass = INT_MAX;
	float xform[16];
	struct prop *prop;

	if(!renderer[REND]->rendpass[rpass]) {
		return;
	}

	pcell = lvl.cells + player.cy * lvl.width + player.cx;
	num_vis = darr_size(pcell->vis);
	for(i=0; i<num_vis; i++) {
		cell = pcell->vis[i];

		/* during the geometry pass, skip cells behind the player */
		if(rpass == RPASS_GEOM && !cell_infront(&player, cell->x, cell->y)) {
			continue;
		}
		count++;

		cgm_mtranslation(world_matrix, cell->x * lvl.cell_size, 0, -cell->y * lvl.cell_size);

		glPushMatrix();
		glMultMatrixf(world_matrix);

		if(cell->tile) {
			cgm_mrotation_y(xform, cell->tilerot * M_PI / 2.0f);

			glPushMatrix();
			glMultMatrixf(xform);
			if(rpass < last_rpass) {
				upd_scene_xform(&cell->tile->scn, time_msec);
			}
			rend_pass(REND, rpass, &cell->tile->scn);
			glPopMatrix();

			prop = cell->props;
			while(prop) {
				if(rpass < last_rpass) {
					upd_scene_xform(&prop->scn, time_msec);
				}
				rend_pass(REND, rpass, &prop->scn);
				prop = prop->next;
			}
		}

		if(rpass < last_rpass) {
			upd_scene_xform(&cell->scn, time_msec);
		}
		rend_pass(REND, rpass, &cell->scn);
		glPopMatrix();
	}

	last_rpass = rpass;

	dbg_printf("vis (pass: %s): %d\n", rpass_name[rpass], count);
}


static void resize(int x, int y)
{
	resize_rtarg(&rtarg, x, y);
	vp_width = x;
	vp_height = y;
	vp_aspect = win_aspect;
}

static void keyboard(int key, int press)
{
}

static void mbutton(int bn, int press, int x, int y)
{
}

static void mmotion(int x, int y)
{
}
