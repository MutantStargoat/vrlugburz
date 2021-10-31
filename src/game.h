#ifndef GAME_H_
#define GAME_H_

#define NEAR_CLIP	0.5f

enum { DIR_N, DIR_E, DIR_S, DIR_W };

enum {
	INP_LEFT,
	INP_RIGHT,
	INP_FWD,
	INP_BACK,
	INP_LTURN,
	INP_RTURN,

	MAX_INP
};

enum {
	KEY_LEFT = 256,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_PGUP,
	KEY_PGDOWN
};

long time_msec;
int input_state[MAX_INP];

int win_width, win_height;
float win_aspect;

float world_matrix[16], view_matrix[16], proj_matrix[16];

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
