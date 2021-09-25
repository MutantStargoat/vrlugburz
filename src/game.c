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

#define STEP_INTERVAL	128

void update(float dt)
{
	static long prev_step;
	int dir;
	int step[][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};

	cgm_vec3 vdir = {0, 0, -1};

	cgm_vmul_m3v3(&vdir, player.view_xform);

	player.dir = (int)(2.0f * (-atan2(vdir.z, vdir.x) + M_PI) / M_PI + 0.5f) & 3;

	if(time_msec - prev_step >= STEP_INTERVAL) {
		if(input_state[INP_FWD]) {
			player.cx += step[player.dir][0];
			player.cy += step[player.dir][1];
			prev_step = time_msec;
			printf("step[%d] %d,%d\n", player.dir, player.cx, player.cy);
		}
		if(input_state[INP_BACK]) {
			player.cx -= step[player.dir][0];
			player.cy -= step[player.dir][1];
			prev_step = time_msec;
			printf("step[%d] %d,%d\n", player.dir, player.cx, player.cy);
		}
		if(input_state[INP_LEFT]) {
			dir = (player.dir + 3) & 3;
			player.cx += step[dir][0];
			player.cy += step[dir][1];
			prev_step = time_msec;
			printf("step[%d] %d,%d\n", player.dir, player.cx, player.cy);
		}
		if(input_state[INP_RIGHT]) {
			dir = (player.dir + 1) & 3;
			player.cx += step[dir][0];
			player.cy += step[dir][1];
			prev_step = time_msec;
			printf("step[%d] %d,%d\n", player.dir, player.cx, player.cy);
		}
		memset(input_state, 0, sizeof input_state);
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
	cgm_mperspective(proj_matrix, cgm_deg_to_rad(50), win_aspect, 0.5, 500.0);
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
	int i, j, k;
	struct cell *cell;
	float xform[16];

	glUseProgram(sdr_foo);

	cell = lvl.cells;
	for(i=0; i<lvl.height; i++) {
		for(j=0; j<lvl.width; j++) {
			cgm_mtranslation(xform, j * lvl.cell_size, 0, -i * lvl.cell_size);

			glPushMatrix();
			glMultMatrixf(xform);

			if(cell->tile) {
				cgm_mrotation_y(xform, cell->tilerot * M_PI / 2.0f);

				glPushMatrix();
				glMultMatrixf(xform);
				draw_meshgroup(&cell->tile->mgrp);
				glPopMatrix();
			}

			for(k=0; k<cell->num_mgrp; k++) {
				draw_meshgroup(cell->mgrp + k);
			}
			cell++;

			glPopMatrix();
		}
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

	switch(key) {
	case 'w':
		input_state[INP_FWD] = press;
		break;

	case 'a':
		input_state[INP_LEFT] = press;
		break;

	case 's':
		input_state[INP_BACK] = press;
		break;

	case 'd':
		input_state[INP_RIGHT] = press;
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
		player.theta -= cgm_deg_to_rad(dx * 0.5f);
		player.phi -= cgm_deg_to_rad(dy * 0.5f);
		if(player.phi < -M_PI/2) player.phi = -M_PI/2;
		if(player.phi > M_PI/2) player.phi = M_PI/2;
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}
}
