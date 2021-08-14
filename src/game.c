#include <assert.h>
#include "game.h"
#include "opengl.h"

int game_init(void)
{
	if(init_opengl() == -1) {
		return -1;
	}
	return 0;
}

void game_shutdown(void)
{
}

void game_display(void)
{
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	game_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
}

void game_reshape(int x, int y)
{
	glViewport(0, 0, x, y);
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
}

void game_mmotion(int x, int y)
{
}
