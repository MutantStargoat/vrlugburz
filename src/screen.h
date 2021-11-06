#ifndef SCREEN_H_
#define SCREEN_H_

struct screen {
	char *name;

	int (*init)(void);
	void (*cleanup)(void);

	int (*start)(void);
	void (*stop)(void);

	void (*update)(float dt);
	void (*draw)(void);

	void (*resize)(int x, int y);

	void (*keyboard)(int key, int press);
	void (*mbutton)(int bn, int press, int x, int y);
	void (*mmotion)(int x, int y);

	struct screen *next;
};

#define MAX_SCREENS	16
struct screen *screens[MAX_SCREENS];
int num_screens;

struct screen *curscr;

int init_all_screens(void);
void destroy_all_screens(void);

struct screen *find_screen(const char *name);
void start_screen(struct screen *scr);

#endif	/* SCREEN_H_ */
