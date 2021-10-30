#ifndef TEXTURE_H_
#define TEXTURE_H_

enum { TEX_2D, TEX_CUBE };

struct texture {
	char *name;
	int type, pixfmt;
	unsigned int tex;
	int width, height;
	void *pixels;
};

int load_texture(struct texture *tex, const char *fname);
void destroy_texture(struct texture *tex);

struct texture *get_texture(const char *fname);

#endif	/* TEXTURE_H_ */
