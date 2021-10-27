#include <stdio.h>
#include <assert.h>
#include "cgmath/cgmath.h"
#include "game.h"
#include "opengl.h"
#include "level.h"
#include "player.h"
#include "rend.h"

static void draw_level(int rpass);

struct level lvl;
struct player player;

int win_width, win_height;
float win_aspect;
int mouse_x, mouse_y;
int bnstate[8];

float cam_dist;
float view_matrix[16], proj_matrix[16];

static long prev_step, prev_turn;

static int rend = REND_LEVEL;

int game_init(void)
{
	if(init_opengl() == -1) {
		return -1;
	}

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if(rend_init() == -1) {
		return -1;
	}

	if(load_level(&lvl, "data/test.lvl") == -1) {
		return -1;
	}

	init_player(&player);
	player.lvl = &lvl;
	player.cx = lvl.px;
	player.cy = lvl.py;

	return 0;
}

void game_shutdown(void)
{
	rend_destroy();
	destroy_level(&lvl);
	free_all_tilesets();
}

#define STEP_INTERVAL	250
#define TURN_INTERVAL	500

void update(float dt)
{
	int upd_dir_pending = 1;
	int fwd = 0, right = 0, turn = 0;

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
	float dt;
	static long prev_msec;

	dt = (prev_msec - time_msec) / 1000.0f;
	prev_msec = time_msec;

	update(dt);

	glClearColor(0.1, 0.1, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cgm_midentity(proj_matrix);
	cgm_mperspective(proj_matrix, cgm_deg_to_rad(60), win_aspect, 0.5, 500.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_matrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glMultMatrixf(player.view_xform);

	rend_begin(rend, RPASS_GEOM);
	draw_level(RPASS_GEOM);
	rend_end(rend, RPASS_GEOM);
	rend_begin(rend, RPASS_LIGHT);
	draw_level(RPASS_LIGHT);
	rend_end(rend, RPASS_LIGHT);

	game_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void draw_level(int rpass)
{
	struct cell *cell;
	float xform[16];
	static int last_rpass = INT_MAX;
	struct prop *prop;

	if(!renderer[rend]->rendpass[rpass]) {
		return;
	}

	if(!player.vis) {
		upd_player_vis(&player);
	}

	cell = player.vis;
	while(cell) {
		cgm_mtranslation(xform, cell->x * lvl.cell_size, 0, -cell->y * lvl.cell_size);

		glPushMatrix();
		glMultMatrixf(xform);

		if(cell->tile) {
			cgm_mrotation_y(xform, cell->tilerot * M_PI / 2.0f);

			glPushMatrix();
			glMultMatrixf(xform);
			if(rpass < last_rpass) {
				upd_scene_xform(&cell->tile->scn, time_msec);
			}
			rend_pass(rend, rpass, &cell->tile->scn);

			prop = cell->props;
			while(prop) {
				if(rpass < last_rpass) {
					upd_scene_xform(&prop->scn, time_msec);
				}
				rend_pass(rend, rpass, &prop->scn);
				prop = prop->next;
			}
			glPopMatrix();
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
