#include <stdio.h>
#include <assert.h>
#include "cgmath/cgmath.h"
#include "game.h"
#include "opengl.h"
#include "level.h"
#include "player.h"
#include "scenefile.h"
#include "sdr.h"

static void draw_level(void);

struct level lvl;
struct player player;

int win_width, win_height;
float win_aspect;
int mouse_x, mouse_y;
int bnstate[8];

float cam_dist;
float view_matrix[16], proj_matrix[16];

unsigned int sdr_foo;

static long prev_step, prev_turn;

int game_init(void)
{
	if(init_opengl() == -1) {
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if(!(sdr_foo = create_program_load("sdr/foo.v.glsl", "sdr/foo.p.glsl"))) {
		return -1;
	}
	glBindAttribLocation(sdr_foo, MESH_ATTR_VERTEX, "apos");
	glBindAttribLocation(sdr_foo, MESH_ATTR_NORMAL, "anorm");
	glBindAttribLocation(sdr_foo, MESH_ATTR_TANGENT, "atang");
	glBindAttribLocation(sdr_foo, MESH_ATTR_TEXCOORD, "atex");
	link_program(sdr_foo);

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
	destroy_level(&lvl);
	free_program(sdr_foo);
}

#define STEP_INTERVAL	250
#define TURN_INTERVAL	500

void update(float dt)
{
	int fwd = 0, right = 0, turn = 0;

	if(time_msec - prev_turn >= TURN_INTERVAL) {
		if(input_state[INP_LTURN]) turn--;
		if(input_state[INP_RTURN]) turn++;

		if(turn) {
			turn_player(&player, turn);
			prev_turn = time_msec;
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
		}
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
	cgm_mperspective(proj_matrix, cgm_deg_to_rad(80), win_aspect, 0.5, 500.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_matrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glMultMatrixf(player.view_xform);

	draw_level();

	game_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void draw_level(void)
{
	int i;
	struct cell *cell;
	float xform[16];

	glUseProgram(sdr_foo);

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
			draw_meshgroup(&cell->tile->mgrp);
			glPopMatrix();
		}

		for(i=0; i<cell->num_mgrp; i++) {
			draw_meshgroup(cell->mgrp + i);
		}
		glPopMatrix();

		cell = cell->next;
	}

	glUseProgram(0);
}

void game_reshape(int x, int y)
{
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
