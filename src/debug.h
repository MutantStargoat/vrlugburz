#ifndef DEBUG_H_
#define DEBUG_H_

int dbg_init(void);
void dbg_begin(void);
void dbg_end(void);
void dbg_printf(const char *fmt, ...);

#endif	/* DEBUG_H_ */
