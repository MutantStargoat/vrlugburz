#include <stdio.h>
#include <stdlib.h>
#include "opt.h"
#include "optcfg.h"

enum {
	OPTCFG_SIZE,
	OPTCFG_VR,
	OPTCFG_FULLSCREEN,
	OPTCFG_VSYNC,
	OPTCFG_HELP
};

static struct optcfg_option optlist[] = {
	{'s', "size", OPTCFG_SIZE, "window size (WxH)"},
	{0, "vr", OPTCFG_VR, "enable VR mode"},
	{'f', "fullscreen", OPTCFG_FULLSCREEN, "start fullscreen"},
	{0, "vsync", OPTCFG_VSYNC, "enable vsync"},
	{'h', "help", OPTCFG_HELP, "print usage and exit"},
	OPTCFG_OPTIONS_END
};

static int opt_handler(struct optcfg *oc, int opt, void *cls);
static int arg_handler(struct optcfg *oc, const char *arg, void *cls);

static char *argv0;

int init_options(int argc, char **argv, const char *cfgfile)
{
	struct optcfg *oc;

	/* default options */
	opt.width = 1280;
	opt.height = 800;
	opt.flags = OPT_VSYNC;

	argv0 = argv[0];

	oc = optcfg_init(optlist);
	optcfg_set_opt_callback(oc, opt_handler, 0);
	optcfg_set_arg_callback(oc, arg_handler, 0);

	if(cfgfile) {
		optcfg_parse_config_file(oc, cfgfile);
	}

	if(argv && optcfg_parse_args(oc, argc, argv) == -1) {
		optcfg_destroy(oc);
		return -1;
	}

	optcfg_destroy(oc);
	return 0;
}

static int is_enabled(struct optcfg *oc)
{
	int res;
	optcfg_enabled_value(oc, &res);
	printf("%s\n", res ? "enabled" : "disabled");
	return res != 0;
}

static int opt_handler(struct optcfg *oc, int optid, void *cls)
{
	char *valstr;

	if(optid != OPTCFG_HELP) {
		if(optlist[optid].s) {
			printf("option %s: ", optlist[optid].s);
		} else {
			printf("option %c: ", optlist[optid].c);
		}
	}

	switch(optid) {
	case OPTCFG_SIZE:
		valstr = optcfg_next_value(oc);
		if(!valstr || sscanf(valstr, "%dx%d", &opt.width, &opt.height) != 2) {
			fprintf(stderr, "size must be of the form: WIDTHxHEIGHT\n");
			return -1;
		}
		printf("%dx%d\n", opt.width, opt.height);
		break;

	case OPTCFG_VR:
		if(is_enabled(oc)) {
			opt.flags |= OPT_VR;
		} else {
			opt.flags &= ~OPT_VR;
		}
		break;

	case OPTCFG_FULLSCREEN:
		if(is_enabled(oc)) {
			opt.flags |= OPT_FULLSCREEN;
		} else {
			opt.flags &= ~OPT_FULLSCREEN;
		}
		break;

	case OPTCFG_VSYNC:
		if(is_enabled(oc)) {
			opt.flags |= OPT_VSYNC;
		} else {
			opt.flags &= ~OPT_VSYNC;
		}
		break;

	case OPTCFG_HELP:
		printf("Usage: %s [options]\nOptions:\n", argv0);
		optcfg_print_options(oc);
		exit(0);
	}
	return 0;
}

static int arg_handler(struct optcfg *oc, const char *arg, void *cls)
{
	fprintf(stderr, "unexpected argument: %s\n", arg);
	return -1;
}
