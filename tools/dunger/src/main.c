#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <utk/cubertk.h>
#include <drawtext.h>
#include "level.h"
#include "lview.h"

static int init(void);
static void cleanup(void);
static void display(void);
static void reshape(int x, int y);
static void keyb(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static void cb_new(utk_event *ev, void *data);
static void cb_open(utk_event *ev, void *data);
static void cb_save(utk_event *ev, void *data);

static void cb_cancel(utk_event *ev, void *data);
static void cb_new_ok(utk_event *ev, void *data);

static int parse_args(int argc, char **argv);


static void ucolor(int r, int g, int b, int a);
static void uclip(int x1, int y1, int x2, int y2);
static void uimage(int x, int y, const void *pix, int xsz, int ysz);
static void urect(int x1, int y1, int x2, int y2);
static void uline(int x1, int y1, int x2, int y2, int width);
static void utext(int x, int y, const char *txt, int sz);
static int utextspacing(void);
static int utextwidth(const char *txt, int sz);


int win_width, win_height;
int view_width, view_height;
float view_panx, view_pany, view_zoom = 1.0f;

static int bnstate[8];
int mousex, mousey, clickx, clicky;

static float uiscale = 1.0f;
#define UISPLIT	150
int splitx;

#define FONTSZ	16
static struct dtx_font *uifont;
static utk_widget *uiroot, *uiwin_new;

static struct level *lvl;


int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	glutInitWindowSize(1280, 800);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutCreateWindow("dunger");

	win_width = glutGet(GLUT_WINDOW_WIDTH);
	win_height = glutGet(GLUT_WINDOW_HEIGHT);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutKeyboardUpFunc(keyup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	if(init() == -1) {
		return 1;
	}
	atexit(cleanup);

	glutMainLoop();
	return 0;
}


static int init(void)
{
	utk_widget *win, *box;

	glEnable(GL_MULTISAMPLE);

	glClearColor(0.15, 0.15, 0.15, 1);

	if(!(uifont = dtx_open_font("uifont.ttf", 0))) {
		fprintf(stderr, "failed to open uifont.ttf\n");
		return -1;
	}
	dtx_prepare_range(uifont, FONTSZ, ' ', 'z');
	dtx_use_font(uifont, FONTSZ);

	if(!(uiroot = utk_init(win_width / uiscale, win_height / uiscale))) {
		fprintf(stderr, "failed to initialized ubertk\n");
		return -1;
	}
	utk_set_color_func(ucolor);
	utk_set_clip_func(uclip);
	utk_set_image_func(uimage);
	utk_set_rect_func(urect);
	utk_set_line_func(uline);
	utk_set_text_func(utext);
	utk_set_text_spacing_func(utextspacing);
	utk_set_text_width_func(utextwidth);

	win = utk_vbox(uiroot, 0, UTK_DEF_SPACING);
	utk_set_pos(win, 15, 15);
	utk_button(win, "New", 0, 0, cb_new, 0);
	utk_button(win, "Open ...", 0, 0, cb_open, 0);
	utk_button(win, "Save ...", 0, 0, cb_save, 0);

	uiwin_new = utk_window(uiroot, 10, 10, 300, 200, "New level");
	{
		const char *items[] = {"small (8x8)", "medium (16x16)", "large (32x32)"};
		utk_combobox_items(uiwin_new, items, sizeof items / sizeof *items, 0, 0);
	}
	box = utk_hbox(uiwin_new, UTK_DEF_PADDING, UTK_DEF_SPACING);
	utk_button(box, "OK", 0, 0, cb_new_ok, 0);
	utk_button(box, "Cancel", 0, 0, cb_cancel, uiwin_new);

	if(!(lvl = create_level(32, 32))) {
		fprintf(stderr, "failed to create level\n");
		return -1;
	}
	if(init_lview(lvl) == -1) {
		return -1;
	}

	splitx = UISPLIT * uiscale;
	view_width = win_width - splitx;
	view_height = win_height;

	return 0;
}

static void cleanup(void)
{
	destroy_lview();
	free_level(lvl);
	dtx_close_font(uifont);
	utk_close(uiroot);
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* draw UI */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, splitx, win_height, 0, -1, 1);
	glViewport(0, 0, splitx, win_height);

	glBegin(GL_QUADS);
	glColor3f(0.25, 0.25, 0.25);
	glVertex2f(0, 0);
	glVertex2f(splitx, 0);
	glVertex2f(splitx, win_height);
	glVertex2f(0, win_height);
	glEnd();
	utk_draw(uiroot);

	/* draw view */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, view_width, 0, view_height, -1, 1);
	glViewport(splitx, 0, view_width, view_height);

	glBegin(GL_QUADS);
	glColor3f(0.1, 0.1, 0.1);
	glVertex2f(0, 0);
	glVertex2f(view_width, 0);
	glVertex2f(view_width, view_height);
	glVertex2f(0, view_height);
	glEnd();

	draw_lview();

	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;

	if(uiroot) {
		utk_set_size(uiroot, x / uiscale, y / uiscale);
	}

	lview_viewport(splitx, 0, x - splitx, y);
}

static void keyb(unsigned char key, int x, int y)
{
	if(key == 27) exit(0);
	utk_keyboard_event(key, 1);
	glutPostRedisplay();
}

static void keyup(unsigned char key, int x, int y)
{
	utk_keyboard_event(key, 0);
	glutPostRedisplay();
}

static void mouse(int bn, int st, int x, int y)
{
	int bidx = bn - GLUT_LEFT_BUTTON;
	int press = st == GLUT_DOWN;

	bnstate[bidx] = press;
	mousex = x;
	mousey = y;

	if(bn <= 2) {
		if(press) {
			clickx = x;
			clicky = y;
		} else {
			clickx = clicky = -1;
		}
	} else if(bn == 3) {
		if(press) view_zoom += 0.1;
	} else if(bn == 4) {
		if(press) view_zoom -= 0.1;
	}

	lview_mbutton(bidx, press, x, y);

	utk_mbutton_event(bidx, press, x / uiscale, y / uiscale);
	glutPostRedisplay();
}

static void motion(int x, int y)
{
	int dx, dy;
	dx = x - mousex;
	dy = y - mousey;
	mousex = x;
	mousey = y;

	if(clickx >= splitx) {
		if(bnstate[1]) {
			view_panx -= dx;
			view_pany += dy;
		}
	}

	lview_mouse(x, y);

	utk_mmotion_event(x / uiscale, y / uiscale);
	glutPostRedisplay();
}

static void cb_new(utk_event *ev, void *data)
{
	utk_show(uiwin_new);
}

static void cb_open(utk_event *ev, void *data)
{
}

static void cb_save(utk_event *ev, void *data)
{
}

static void cb_cancel(utk_event *ev, void *data)
{
	utk_hide(data);
}

static void cb_new_ok(utk_event *ev, void *data)
{
}

static int parse_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-uiscale") == 0) {
				if(!argv[++i] || !(uiscale = atoi(argv[i]))) {
					fprintf(stderr, "-uiscale should be followed by a positive number\n");
					return -1;
				}
			} else if(strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0) {
				printf("Usage: %s [options]\n", argv[0]);
				printf("Options:\n");
				printf(" -uiscale <scale>: UI scale factor (default: 1)\n");
				printf(" -h,-help: print usage and exit\n");
				exit(0);
			} else {
				fprintf(stderr, "unknown option: %s\n", argv[i]);
				return -1;
			}
		} else {
			fprintf(stderr, "unexpected argument: %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}

/* --- ubertk callbacks --- */

static void ucolor(int r, int g, int b, int a)
{
	glColor4ub(r, g, b, a);
}

static void uclip(int x1, int y1, int x2, int y2)
{
	if(!(x1 | y1 | x2 | y2)) {
		glDisable(GL_SCISSOR_TEST);
	} else {
		glEnable(GL_SCISSOR_TEST);
	}

	x1 *= uiscale;
	y1 *= uiscale;
	x2 *= uiscale;
	y2 *= uiscale;

	glScissor(x1, win_height - y2, x2 - x1, y2 - y1);
}

static void uimage(int x, int y, const void *pix, int xsz, int ysz)
{
	glPixelZoom(1, -1);
	glRasterPos2f(x * uiscale, y * uiscale);
	glDrawPixels(xsz, ysz, GL_BGRA, GL_UNSIGNED_BYTE, pix);
}

static void urect(int x1, int y1, int x2, int y2)
{
	glRectf(x1 * uiscale, y1 * uiscale, x2 * uiscale, y2 * uiscale);
}

static void uline(int x1, int y1, int x2, int y2, int width)
{
	glLineWidth(width);
	glBegin(GL_LINES);
	glVertex2f(x1 * uiscale, y1 * uiscale);
	glVertex2f(x2 * uiscale, y2 * uiscale);
	glEnd();
}

static void utext(int x, int y, const char *txt, int sz)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glTranslatef(x * uiscale, (y - dtx_baseline()) * uiscale, 0);
	glScalef(uiscale, -uiscale, 1);

	dtx_string(txt);
	dtx_flush();

	glPopMatrix();
}

static int utextspacing(void)
{
	return dtx_line_height();
}

static int utextwidth(const char *txt, int sz)
{
	return dtx_string_width(txt);
}
