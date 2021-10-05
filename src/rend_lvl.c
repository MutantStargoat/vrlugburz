#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"
#include "rend.h"
#include "scene.h"

static void destroy(void);
static void reshape(int x, int y);
static void begin(int pass);
static void end(int pass);
static void shadow_pass(struct scene *scn);
static void geom_pass(struct scene *scn);
static void light_pass(struct scene *scn);
static void blend_pass(struct scene *scn);

static struct renderer rfunc = {
	destroy,
	reshape,
	begin, end,
	{ shadow_pass, geom_pass, light_pass, blend_pass }
};

struct renderer *init_rend_level(void)
{
	return &rfunc;
}

static void destroy(void)
{
}

static void reshape(int x, int y)
{
}

static void begin(int pass)
{
}

static void end(int pass)
{
}

static void shadow_pass(struct scene *scn)
{
}

static void geom_pass(struct scene *scn)
{
}

static void light_pass(struct scene *scn)
{
}

static void blend_pass(struct scene *scn)
{
}
