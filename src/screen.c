#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "screen.h"

int init_all_screens(void)
{
	int i;
	for(i=0; i<num_screens; i++) {
		if(screens[i]->init && screens[i]->init() == -1) {
			return -1;
		}
	}
	return 0;
}

void destroy_all_screens(void)
{
	int i;
	for(i=0; i<num_screens; i++) {
		if(screens[i]->cleanup) {
			screens[i]->cleanup();
		}
	}
}

struct screen *find_screen(const char *name)
{
	int i;
	if(!name) return 0;
	for(i=0; i<num_screens; i++) {
		if(strcmp(screens[i]->name, name) == 0) {
			return screens[i];
		}
	}
	return 0;
}

void start_screen(struct screen *scr)
{
	assert(num_screens > 0);
	if(!scr) scr = screens[0];
	if(scr == curscr) return;

	if(curscr) {
		printf("stopping screen: %s\n", curscr->name);
		if(curscr->stop) {
			curscr->stop();
		}
	}
	curscr = scr;
	printf("starting screen: %s\n", scr->name);
	if(scr->start) {
		scr->start();
	}
}
