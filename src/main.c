#include <stdio.h>
#include <stdlib.h>
#include "miniglut.h"
#include "opengl.h"
#include "game.h"
#include "opt.h"

static void display(void);
static void idle(void);
static void keypress(unsigned char key, int x, int y);
static void keyrelease(unsigned char key, int x, int y);
static void skeypress(int key, int x, int y);
static void skeyrelease(int key, int x, int y);
static void mbutton(int bn, int st, int x, int y);
static int skey_translate(int key);

static int quit;
static long start_time;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1280, 720);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_SRGB);
	glutCreateWindow("lugburz VR");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(game_reshape);
	glutKeyboardFunc(keypress);
	glutKeyboardUpFunc(keyrelease);
	glutSpecialFunc(skeypress);
	glutSpecialUpFunc(skeyrelease);
	glutMouseFunc(mbutton);
	glutMotionFunc(game_mmotion);
	glutPassiveMotionFunc(game_mmotion);

	glutIgnoreKeyRepeat(1);

	if(init_options(argc, argv, "game.cfg") == -1) {
		return 1;
	}

	if(game_init() == -1) {
		return 1;
	}

	start_time = glutGet(GLUT_ELAPSED_TIME);
	while(!quit) {
		glutMainLoopEvent();
	}

	game_shutdown();
	return 0;
}

void game_quit(void)
{
	quit = 1;
}

void game_swap_buffers(void)
{
	glutSwapBuffers();
}

static void display(void)
{
	time_msec = glutGet(GLUT_ELAPSED_TIME) - start_time;
	game_display();
}

static void idle(void)
{
	glutPostRedisplay();
}

static void keypress(unsigned char key, int x, int y)
{
	game_keyboard(key, 1);
}

static void keyrelease(unsigned char key, int x, int y)
{
	game_keyboard(key, 0);
}

static void skeypress(int key, int x, int y)
{
	if((key = skey_translate(key)) >= 0) {
		game_keyboard(key, 1);
	}
}

static void skeyrelease(int key, int x, int y)
{
	if((key = skey_translate(key)) >= 0) {
		game_keyboard(key, 0);
	}
}

static void mbutton(int bn, int st, int x, int y)
{
	int bidx = bn - GLUT_LEFT_BUTTON;
	int press = st == GLUT_DOWN;
	game_mbutton(bidx, press, x, y);
}

static int skey_translate(int key)
{
	switch(key) {
	case GLUT_KEY_LEFT:
		return KEY_LEFT;
	case GLUT_KEY_RIGHT:
		return KEY_RIGHT;
	case GLUT_KEY_UP:
		return KEY_UP;
	case GLUT_KEY_DOWN:
		return KEY_DOWN;
	case GLUT_KEY_PAGE_UP:
		return KEY_PGUP;
	case GLUT_KEY_PAGE_DOWN:
		return KEY_PGDOWN;
	default:
		return -1;
	}
}
