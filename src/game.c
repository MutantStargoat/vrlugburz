#include <assert.h>
#include "cgmath/cgmath.h"
#include "game.h"
#include "opengl.h"
#include "level.h"
#include "scenefile.h"
#include "sdr.h"

struct level lvl;
struct scenefile scn;

int win_width, win_height;
float win_aspect;
int mouse_x, mouse_y;
int bnstate[8];

float cam_theta, cam_phi, cam_dist = 10;
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

	/* DBG */
	if(load_scenefile(&scn, "data/dwall1.obj") == -1) {
		return -1;
	}

	return 0;
}

void game_shutdown(void)
{
	destroy_level(&lvl);
	free_program(sdr_foo);
}

void game_display(void)
{
	struct mesh *mesh;

	glClearColor(0.1, 0.1, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cgm_midentity(proj_matrix);
	cgm_mperspective(proj_matrix, cgm_deg_to_rad(50), win_aspect, 0.5, 500.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_matrix);

	cgm_midentity(view_matrix);
	cgm_mpretranslate(view_matrix, 0, 0, -cam_dist);
	cgm_mprerotate(view_matrix, cam_phi, 1, 0, 0);
	cgm_mprerotate(view_matrix, cam_theta, 0, 1, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view_matrix);

	glUseProgram(sdr_foo);

	mesh = scn.meshlist;
	while(mesh) {
		draw_mesh(mesh);
		mesh = mesh->next;
	}

	glUseProgram(0);

	game_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
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
		cam_theta += cgm_deg_to_rad(dx * 0.5f);
		cam_phi += cgm_deg_to_rad(dy * 0.5f);
		if(cam_phi < -M_PI/2) cam_phi = -M_PI/2;
		if(cam_phi > M_PI/2) cam_phi = M_PI/2;
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}
}
