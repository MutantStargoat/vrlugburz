#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>

#define GROW_ARRAY(arr, sz, onfail)	\
	do { \
		int newsz = (sz) ? (sz) * 2 : 16; \
		void *tmp = realloc(arr, newsz * sizeof *(arr)); \
		if(!tmp) { \
			fprintf(stderr, "failed to grow array to %d\n", newsz); \
			onfail; \
		} \
		arr = tmp; \
		sz = newsz; \
	} while(0)


#endif	/* UTIL_H_ */
