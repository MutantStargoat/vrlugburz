#ifndef GAME_H_
#define GAME_H_

enum {
	KEY_LEFT = 256,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_PGUP,
	KEY_PGDOWN
};

long time_msec;

int game_init(void);
void game_shutdown(void);

void game_display(void);
void game_reshape(int x, int y);
void game_keyboard(int key, int press);
void game_mbutton(int bn, int press, int x, int y);
void game_mmotion(int x, int y);

void game_quit(void);
void game_swap_buffers(void);

#endif	/* GAME_H_ */
