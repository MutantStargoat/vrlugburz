#ifndef OPT_H_
#define OPT_H_


enum {
	OPT_VR			= 1,
	OPT_FULLSCREEN	= 2,
	OPT_VSYNC		= 4
};

struct options {
	int width, height;
	unsigned int flags;
	char *start_scr;
} opt;

int init_options(int argc, char **argv, const char *cfgfile);

#endif	/* OPT_H_ */
