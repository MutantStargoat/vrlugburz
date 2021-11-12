#include <stdio.h>
#include <stdarg.h>
#include "opengl.h"
#include "game.h"
#include "drawtext.h"
#include "debug.h"

#define FONT_SIZE	20
static struct dtx_font *font;
static int line_height;

#define MAX_LINES	25
#define ROW_SIZE	128
static char lines[MAX_LINES][ROW_SIZE];
static int curline;

int dbg_init(void)
{
	if(!(font = dtx_open_font_glyphmap("data/dbgfont.glyphmap"))) {
		fprintf(stderr, "failed to load debug output font\n");
		return -1;
	}
	dtx_prepare_range(font, FONT_SIZE, 32, 126);
	dtx_use_font(font, FONT_SIZE);
	line_height = dtx_line_height();
	return 0;
}

void dbg_begin(void)
{
	curline = 0;
}

void dbg_end(void)
{
	int i;

	if(!font) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1024 * vp_aspect, 0, 1024, -1, 1);

	glColor3f(1, 1, 1);
	dtx_use_font(font, FONT_SIZE);
	dtx_draw_buffering(DTX_FBF);
	for(i=0; i<curline; i++) {
		dtx_string(lines[i]);
		glTranslatef(0, line_height, 0);
	}
	dtx_flush();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void dbg_printf(const char *fmt, ...)
{
	va_list ap;

	if(!font || curline >= MAX_LINES) return;

	va_start(ap, fmt);
	vsnprintf(lines[curline++], ROW_SIZE, fmt, ap);
	va_end(ap);
}
