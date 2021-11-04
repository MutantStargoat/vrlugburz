#include <stdio.h>
#include <assert.h>
#include "cgmath/cgmath.h"
#include "game.h"
#include "opengl.h"
#include "level.h"
#include "player.h"
#include "rend.h"
#include "rtarg.h"
#include "sdr.h"
#include "vr.h"

static void render_game(struct render_target *fbrt);
static void draw_level(int rpass);

struct level lvl;
struct player player;

int win_width, win_height;
float win_aspect;
int mouse_x, mouse_y;
int bnstate[8];

float cam_dist;

static long prev_step, prev_turn;

static int rend = REND_LEVEL;

static struct render_target rtarg;
static unsigned int sdr_post;
static int uloc_expose;


int game_init(void)
{
	if(init_opengl() == -1) {
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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

	if(rend_init() == -1) {
		return -1;
	}

	if(init_rtarg(&rtarg, win_width, win_height, 1) == -1) {
		fprintf(stderr, "failed to create %dx%d framebuffer\n", win_width, win_height);
		return -1;
	}

	if(load_level(&lvl, "data/test.lvl") == -1) {
		return -1;
	}

	init_player(&player);
	player.lvl = &lvl;
	player.cx = lvl.px;
	player.cy = lvl.py;

	if(init_vr() == -1) {
		return -1;
	}

	return 0;
}

void game_shutdown(void)
{
	rend_destroy();
	destroy_level(&lvl);
	free_all_tilesets();
	free_props();
}

#define STEP_INTERVAL	250
#define TURN_INTERVAL	500

void update(float dt)
{
	int upd_dir_pending = 1;
	int fwd = 0, right = 0, turn = 0;

	update_vr_input();

	if(time_msec - prev_turn >= TURN_INTERVAL) {
		if(input_state[INP_LTURN]) turn--;
		if(input_state[INP_RTURN]) turn++;

		if(turn) {
			turn_player(&player, turn);
			prev_turn = time_msec;
			upd_dir_pending = 0;
		}
	}

	if(time_msec - prev_step >= STEP_INTERVAL) {
		if(input_state[INP_FWD]) fwd++;
		if(input_state[INP_BACK]) fwd--;
		if(input_state[INP_LEFT]) right--;
		if(input_state[INP_RIGHT]) right++;

		if(fwd | right) {
			move_player(&player, right, fwd);
			prev_step = time_msec;
			upd_dir_pending = 0;
		}
	}

	if(upd_dir_pending) {
		update_player_dir(&player);
	}
	upd_player_xform(&player);
}

void game_display(void)
{
	int i;
	float dt;
	static long prev_msec;

	dt = (prev_msec - time_msec) / 1000.0f;
	prev_msec = time_msec;

	update(dt);

#ifdef BUILD_VR
	if(goatvr_invr()) {
		unsigned int vrfbo;
		struct render_target vr_rtarg;

		goatvr_draw_start();

		vrfbo = goatvr_get_fbo();

		for(i=0; i<2; i++) {
			goatvr_draw_eye(i);
			if(vrfbo) {
				vr_rtarg.tex_width = vr_rtarg.width = goatvr_get_fb_eye_width(i);
				vr_rtarg.tex_height = vr_rtarg.height = goatvr_get_fb_eye_height(i);
				vr_rtarg.fbo = vrfbo;
			}

			cgm_mcopy(proj_matrix, goatvr_projection_matrix(i, NEAR_CLIP, 500.0f));
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(proj_matrix);

			cgm_mcopy(view_matrix, goatvr_view_matrix(i));
			cgm_mpremul(view_matrix, player.view_xform);
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(view_matrix);

			render_game(vrfbo ? &vr_rtarg : 0);
		}

		goatvr_draw_done();

		if(should_swap) {
			game_swap_buffers();
		}
	} else
#endif	/* BUILD_VR */
	{
		/* non-VR mode */
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

		game_swap_buffers();
	}
}

static void render_game(struct render_target *fbrt)
{
	float sx, sy;

	/* geometry pass */
	rend_begin(rend, RPASS_GEOM);
	draw_level(RPASS_GEOM);
	rend_end(rend, RPASS_GEOM);

	/* light pass */
	bind_rtarg(&rtarg);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	rend_begin(rend, RPASS_LIGHT);
	draw_level(RPASS_LIGHT);
	rend_end(rend, RPASS_LIGHT);

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

	glEnable(GL_DEPTH_TEST);

	assert(glGetError() == GL_NO_ERROR);
}

static void draw_level(int rpass)
{
	struct cell *cell;
	static int last_rpass = INT_MAX;
	float xform[16];
	struct prop *prop;

	if(!renderer[rend]->rendpass[rpass]) {
		return;
	}

	if(!player.vis) {
		upd_player_vis(&player);
	}

	cell = player.vis;
	while(cell) {
		if(rpass != RPASS_LIGHT && !cell->visible) {
			cell = cell->next;
			continue;
		}
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
			rend_pass(rend, rpass, &cell->tile->scn);
			glPopMatrix();

			prop = cell->props;
			while(prop) {
				if(rpass < last_rpass) {
					upd_scene_xform(&prop->scn, time_msec);
				}
				rend_pass(rend, rpass, &prop->scn);
				prop = prop->next;
			}
		}

		if(rpass < last_rpass) {
			upd_scene_xform(&cell->scn, time_msec);
		}
		rend_pass(rend, rpass, &cell->scn);
		glPopMatrix();

		cell = cell->next;
	}

	last_rpass = rpass;
}

void game_reshape(int x, int y)
{
	rend_reshape(x, y);
	resize_rtarg(&rtarg, x, y);
#ifdef BUILD_VR
	goatvr_set_fb_size(x, y, 1.0f);
#endif

	glViewport(0, 0, x, y);
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;
}

void game_keyboard(int key, int press)
{
	if(press && key == 27) {
		game_quit();
		return;
	}

	/* TODO key remapping */
	switch(key) {
	case 'w':
		input_state[INP_FWD] = press;
		if(press) {
			move_player(&player, 0, 1);
			prev_step = time_msec;
		}
		break;

	case 'a':
		input_state[INP_LEFT] = press;
		if(press) {
			move_player(&player, -1, 0);
			prev_step = time_msec;
		}
		break;

	case 's':
		input_state[INP_BACK] = press;
		if(press) {
			move_player(&player, 0, -1);
			prev_step = time_msec;
		}
		break;

	case 'd':
		input_state[INP_RIGHT] = press;
		if(press) {
			move_player(&player, 1, 0);
			prev_step = time_msec;
		}
		break;

	case 'q':
		input_state[INP_LTURN] = press;
		if(press) {
			turn_player(&player, -1);
			prev_turn = time_msec;
		}
		break;

	case 'e':
		input_state[INP_RTURN] = press;
		if(press) {
			turn_player(&player, 1);
			prev_turn = time_msec;
		}
		break;

#ifdef BUILD_VR
	case KEY_HOME:
		if(goatvr_invr()) {
			goatvr_recenter();
		}
		break;
#endif
	}
}

void game_mbutton(int bn, int press, int x, int y)
{
	bnstate[bn] = press;
	mouse_x = x;
	mouse_y = y;
}

void game_mmotion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;

	if(!(dx | dy)) return;

	if(bnstate[0]) {
		player.theta += cgm_deg_to_rad(dx * 0.5f);
		player.phi -= cgm_deg_to_rad(dy * 0.5f);
		if(player.phi < -M_PI/2) player.phi = -M_PI/2;
		if(player.phi > M_PI/2) player.phi = M_PI/2;
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}
}
