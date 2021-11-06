#include <stdio.h>
#include <assert.h>
#include "game.h"
#include "opengl.h"
#include "rend.h"
#include "screen.h"
#include "vr.h"
#include "opt.h"

void reg_game_screen(void);

int win_width, win_height;
float win_aspect;

int game_init(void)
{
	if(init_opengl() == -1) {
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if(init_vr() == -1) {
		return -1;
	}
	if(rend_init() == -1) {
		return -1;
	}

	reg_game_screen();

	if(init_all_screens() == -1) {
		return -1;
	}

	start_screen(find_screen(opt.start_scr));
	return 0;
}

void game_shutdown(void)
{
	rend_destroy();
	destroy_all_screens();
}

void game_display(void)
{
	float dt;
	static long prev_msec;

	if(curscr->update) {
		dt = (prev_msec - time_msec) / 1000.0f;
		prev_msec = time_msec;

		curscr->update(dt);
	}
	curscr->draw();
}

void game_reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;

	rend_reshape(x, y);

	if(curscr->resize) {
		curscr->resize(x, y);
	}
}

void game_keyboard(int key, int press)
{
	if(press && key == 27) {
		game_quit();
		return;
	}

	if(curscr->keyboard) {
		curscr->keyboard(key, press);
	}
}

void game_mbutton(int bn, int press, int x, int y)
{
	if(curscr->mbutton) {
		curscr->mbutton(bn, press, x, y);
	}
}

void game_mmotion(int x, int y)
{
	if(curscr->mmotion) {
		curscr->mmotion(x, y);
	}
}
