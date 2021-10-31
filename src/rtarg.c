#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"
#include "game.h"
#include "rtarg.h"
#include "util.h"


int init_rtarg(struct render_target *rt, int xsz, int ysz, int mrtcount)
{
	int i;

	rt->width = xsz;
	rt->height = ysz;
	rt->tex_width = nextpow2(xsz);
	rt->tex_height = nextpow2(ysz);
	rt->num_tex = mrtcount;

	glGenFramebuffers(1, &rt->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);

	if(mrtcount > 0) {
		glGenTextures(mrtcount, rt->tex);
		for(i=0; i<mrtcount; i++) {
			glBindTexture(GL_TEXTURE_2D, rt->tex[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, rt->tex_width, rt->tex_height, 0,
					GL_RGB, GL_FLOAT, 0);

			rt->owntex |= 1 << i;

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
					GL_TEXTURE_2D, rt->tex[i], 0);
		}
	}

	glGenRenderbuffers(1, &rt->zbuf);
	glBindRenderbuffer(GL_RENDERBUFFER, rt->zbuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, xsz, ysz);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->zbuf);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 0;
}

void destroy_rtarg(struct render_target *rt)
{
	int i;

	if(rt->owntex == rt->num_tex - 1) {
		glDeleteTextures(rt->num_tex, rt->tex);
	} else {
		for(i=0; i<rt->num_tex; i++) {
			if(rt->owntex & 1) {
				glDeleteTextures(1, &rt->tex[i]);
			}
			rt->owntex >>= 1;
		}
	}

	glDeleteRenderbuffers(1, &rt->zbuf);
	glDeleteFramebuffers(1, &rt->fbo);
}

struct render_target *alloc_rtarg(int xsz, int ysz, int mrtcount)
{
	struct render_target *rt = malloc_nf(sizeof *rt);
	if(init_rtarg(rt, xsz, ysz, mrtcount) == -1) {
		free(rt);
		return 0;
	}
	return rt;
}

void free_rtarg(struct render_target *rt)
{
	destroy_rtarg(rt);
	free(rt);
}

int resize_rtarg(struct render_target *rt, int xsz, int ysz)
{
	int i, newxsz, newysz;

	newxsz = nextpow2(xsz);
	newysz = nextpow2(ysz);

	if(newxsz > rt->tex_width || newysz > rt->tex_height) {
		for(i=0; i<rt->num_tex; i++) {
			if(!(rt->owntex & (1 << i))) continue;

			glBindTexture(GL_TEXTURE_2D, rt->tex[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, newxsz, newysz, 0, GL_RGB, GL_FLOAT, 0);
		}
		rt->tex_width = newxsz;
		rt->tex_height = newysz;
	}

	if(xsz > rt->width || ysz > rt->height) {
		glBindRenderbuffer(GL_RENDERBUFFER, rt->zbuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, xsz, ysz);
	}

	rt->width = xsz;
	rt->height = ysz;
	return 0;
}

void bind_rtarg(struct render_target *rt)
{
	static const unsigned int rbuf[] = {
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7
	};

	if(rt) {
		glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
		glViewport(0, 0, rt->width, rt->height);
		glDrawBuffers(rt->num_tex, rbuf);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, win_width, win_height);
	}
}
