#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opengl.h"
#include "imago2.h"
#include "texture.h"
#include "rbtree.h"
#include "util.h"

static struct rbtree *texdb;

static const char *typestr(int type);
static const char *pixfmtstr(int fmt);

int load_texture(struct texture *tex, const char *fname)
{
	struct img_pixmap img;

	img_init(&img);
	if(img_load(&img, fname) == -1) {
		fprintf(stderr, "failed to load texture: %s\n", fname);
		return -1;
	}

	tex->name = strdup_nf(fname);
	tex->type = TEX_2D;
	tex->pixfmt = img_glintfmt_srgb(&img);
	tex->width = img.width;
	tex->height = img.height;
	tex->pixels = img.pixels;

	glGenTextures(1, &tex->tex);
	glBindTexture(GL_TEXTURE_2D, tex->tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, tex->pixfmt, img.width, img.height, 0,
			img_glfmt(&img), img_gltype(&img), img.pixels);

	printf("loaded %s %s: %dx%d %s\n", typestr(tex->type), fname, tex->width,
			tex->height, pixfmtstr(tex->pixfmt));
	return 0;
}

void destroy_texture(struct texture *tex)
{
	if(tex) {
		glDeleteTextures(1, &tex->tex);
		free(tex->pixels);
		free(tex->name);
	}
}

struct texture *get_texture(const char *fname)
{
	struct texture *tex;
	struct rbnode *rbn;

	if(!texdb) {
		if(!(texdb = rb_create(RB_KEY_STRING))) {
			fprintf(stderr, "failed to create texture database\n");
			abort();
		}
	}

	if((rbn = rb_find(texdb, (char*)fname))) {
		return rbn->data;
	}

	tex = malloc_nf(sizeof *tex);
	if(load_texture(tex, fname) == -1) {
		free(tex);
		return 0;
	}

	rb_insert(texdb, tex->name, tex);
	return tex;
}


static const char *typestr(int type)
{
	switch(type) {
	case TEX_2D:
		return "texture";
	case TEX_CUBE:
		return "cubemap";
	default:
		break;
	}
	return "?texture";
}

static const char *pixfmtstr(int fmt)
{
	switch(fmt) {
	case GL_RGB:
		return "RGB";
	case GL_RGBA:
		return "RGBA";
	case GL_LUMINANCE:
		return "grey";
	case GL_SRGB:
		return "sRGB";
	case GL_SRGB_ALPHA:
		return "sRGB+A";
	case GL_SLUMINANCE:
		return "s-grey";
	case GL_RGB16F:
		return "half-float RGB";
	case GL_RGBA16F:
		return "half-float RGBA";
	case GL_LUMINANCE16F_ARB:
		return "half-float grey";
	default:
		break;
	}
	return "unknown";
}
