#ifndef RTARG_H_
#define RTARG_H_

#define RTARG_MAX_COLBUF	8
struct render_target {
	int width, height;
	int tex_width, tex_height;
	int num_tex;
	unsigned int fbo;
	unsigned int tex[RTARG_MAX_COLBUF], zbuf;
	unsigned int owntex;	/* bitmask */
};

int init_rtarg(struct render_target *rt, int xsz, int ysz, int mrtcount);
void destroy_rtarg(struct render_target *rt);

struct render_target *alloc_rtarg(int xsz, int ysz, int mrtcount);
void free_rtarg(struct render_target *rt);

int resize_rtarg(struct render_target *rt, int xsz, int ysz);

void bind_rtarg(struct render_target *rt);

#endif	/* RTARG_H_ */
